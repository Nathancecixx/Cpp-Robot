#include "pch.h"
#include "CppUnitTest.h"

#include "PktDef.h"
#include "MySocket.h"

#include <string>
#include <thread>
#include <chrono>

#define TEST_BUFF_SIZE  1024

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
    namespace VisualStudio {
        namespace CppUnitTestFramework {
            template<>
            std::wstring ToString<CmdType>(const CmdType& q) {
                switch (q) {
                case DRIVE:    return L"DRIVE";
                case SLEEP:    return L"SLEEP";
                case RESPONSE: return L"RESPONSE";
                default:       return L"Unknown CmdType";
                }
            }
            template<>
            std::wstring ToString<SocketType>(const SocketType& st) {
                switch (st) {
                case CLIENT: return L"CLIENT";
                case SERVER: return L"SERVER";
                default:     return L"Unknown SocketType";
                }
            }


        }
    }
}

namespace NetworkRobotTests
{


    /***********************************************************************
     *                         Milestone #1 Tests
     ***********************************************************************/
    TEST_CLASS(UnitTestPktDef)
    {
    public:

        //test 1 - default constructor should initialize to safe state
        TEST_METHOD(DefaultConstructorTest)
        {
            PktDef pkt;
            Assert::AreEqual(0, pkt.GetPktCount());
            Assert::AreEqual(0, pkt.GetLength());
            Assert::IsFalse(pkt.GetAck());
            Assert::IsNull(pkt.GetBodyData());
        }

        //test 2 - testing the setters and getters
        TEST_METHOD(SettersAndGettersTest)
        {
            PktDef pkt;
            pkt.SetPktCount(123);
            Assert::AreEqual(123, pkt.GetPktCount());

            const char* testData = "Hello";
            pkt.SetBodyData(const_cast<char*>(testData), 5);
            // the expected length = HEADERSIZE + 5 + CRCSIZE
            Assert::AreEqual(HEADERSIZE + 5 + CRCSIZE, pkt.GetLength());
            Assert::AreEqual(0, std::memcmp(pkt.GetBodyData(), testData, 5));

            pkt.SetCmd(DRIVE);
            Assert::AreEqual(DRIVE, pkt.GetCmd());
        }

        //test 3 - varify CRC passes for a valid packet
        TEST_METHOD(CRCTest)
        {
            PktDef pkt;
            pkt.SetPktCount(1);
            const char* testData = "ABC";
            pkt.SetBodyData(const_cast<char*>(testData), 3);
            pkt.SetCmd(DRIVE);

            //generate the packet so that RawBuffer is updated
            char* raw = pkt.GenPacket();
            
            //calculate the CRC
            pkt.CalcCRC();
            int totalSize = pkt.GetLength();
            Assert::IsTrue(pkt.CheckCRC(raw, totalSize));
        }

        

        //test 4 - zero length body 
        TEST_METHOD(ZeroLengthBodyTest)
        {
            PktDef pkt;
            pkt.SetCmd(SLEEP);
            pkt.SetPktCount(42);
            pkt.SetBodyData("", 0);

            //the expected length = HEADERSIZE + CRCSIZE
            Assert::AreEqual(HEADERSIZE + 0 + CRCSIZE, pkt.GetLength());
            Assert::IsNotNull(pkt.GetBodyData());
            Assert::AreEqual('\0', pkt.GetBodyData()[0]);
            Assert::AreEqual(SLEEP, pkt.GetCmd());
            Assert::AreEqual(42, pkt.GetPktCount());
        }

        //test 5 - testing large body data for whatever reason
        TEST_METHOD(LargeBodyDataTest)
        {
            PktDef pkt;
            pkt.SetCmd(DRIVE);
            pkt.SetPktCount(999);

            const int bodySize = 50;
            char largeData[bodySize + 1];
            for (int i = 0; i < bodySize; i++)
                largeData[i] = 'X';
            largeData[bodySize] = '\0';

            pkt.SetBodyData(largeData, bodySize);

            Assert::AreEqual(HEADERSIZE + bodySize + CRCSIZE, pkt.GetLength());
            Assert::AreEqual(0, std::memcmp(pkt.GetBodyData(), largeData, bodySize));
            Assert::AreEqual(DRIVE, pkt.GetCmd());
            Assert::AreEqual(999, pkt.GetPktCount());
        }

        //test 6 - testing changing commands in sequance
        TEST_METHOD(MultipleCommandsTest)
        {
            PktDef pkt;
            pkt.SetCmd(DRIVE);
            Assert::AreEqual(DRIVE, pkt.GetCmd());

            pkt.SetCmd(SLEEP);
            Assert::AreEqual(SLEEP, pkt.GetCmd());

            pkt.SetCmd(RESPONSE);
            Assert::AreEqual(RESPONSE, pkt.GetCmd());
        }

        //test 7- testing that setting body data multiple times works
        TEST_METHOD(BodyDataOverwriteTest)
        {
            PktDef pkt;
            pkt.SetCmd(DRIVE);

            const char* data1 = "FirstData";
            pkt.SetBodyData(const_cast<char*>(data1), 9);
            Assert::AreEqual(0, std::memcmp(pkt.GetBodyData(), data1, 9));

            const char* data2 = "SecondData";
            pkt.SetBodyData(const_cast<char*>(data2), 10);
            Assert::AreEqual(0, std::memcmp(pkt.GetBodyData(), data2, 10));
        }

        //test 8 - testing that PktCount wraps with no issues
        TEST_METHOD(OverBoundaryPktCountTest)
        {
            PktDef pkt;
            pkt.SetCmd(DRIVE);
            pkt.SetPktCount(70000);
            // 70000 mod 65536 = 70000 - 65536 = 4464
            Assert::AreEqual(4464, pkt.GetPktCount());
        }

        //test 9 - negative body size handling test for whatever reason
        TEST_METHOD(NegativeBodySizeTest)
        {
            PktDef pkt;
            pkt.SetCmd(SLEEP);
            try {
                pkt.SetBodyData("", -5);
                Assert::IsTrue(pkt.GetLength() >= (HEADERSIZE + CRCSIZE));
            }
            catch (...) {
                Assert::Fail();
            }
        }

    };




    /***********************************************************************
     *                         Milestone #2 Tests
     ***********************************************************************/
    

    //TODO: constructor defaults invalid buffer sizes and dhandles bad ip formats gracefully
    //TODO: No data when send function

     //Helper function to create an TCP stub server that echos messages
    void DummyTCPServer(int port) {
        WSAData wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            return;
        //Init socket
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET)
            return;
        //Set ip info
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) != 1) {
            closesocket(serverSocket);
            WSACleanup();
            return;
        }
        //Bind
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR){
            closesocket(serverSocket);
            WSACleanup();
            return;
        }
        //Listen
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR){
            closesocket(serverSocket);
            WSACleanup();
            return;
        }
        //Accept
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET)
        {
            //Recieve into byffer
            char buffer[TEST_BUFF_SIZE];
            int recvResult = recv(clientSocket, buffer, sizeof(buffer), 0);
            
            //Echo back
            if (recvResult > 0){
                send(clientSocket, buffer, recvResult, 0);
            }
            closesocket(clientSocket);
        }
        closesocket(serverSocket);
        WSACleanup();

    }

    //Helper function to create an UDP stub server that echos messages
    void DummyUDPServer(int port) {
        WSAData wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            return;

        //Init socket
        SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (serverSocket == INVALID_SOCKET)
            return;

        //Set Ip info
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) != 1) {
            closesocket(serverSocket);
            WSACleanup();
            return;
        }
        //Bind
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        //Recieve data into buffer
        char buffer[TEST_BUFF_SIZE] = { 0 };
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientAddrSize);

        //Echo data back
        if (bytesReceived > 0) {
            sendto(serverSocket, buffer, bytesReceived, 0, (sockaddr*)&clientAddr, clientAddrSize);
        }
        closesocket(serverSocket);
        WSACleanup();
    }

    //Test MySocket Class
    TEST_CLASS(UnitTestMySocket)
    {
    public:

        TEST_METHOD(ConstructorTest)
        {
            //Arrange
            std::string ip("127.0.0.1");
            int port = 6000;
            //Act
            MySocket sock(CLIENT, ip, port, TCP, TEST_BUFF_SIZE);
            //Assert
            Assert::AreEqual(CLIENT, sock.GetType());
            Assert::AreEqual(ip, sock.GetIPAddr());
            Assert::AreEqual(port, sock.GetPort());

        }

        //Test setter functions
        TEST_METHOD(SetterPreConnectTest)
        {
            //Arrange
            MySocket sock(CLIENT, "127.0.0.1", 12345, TCP, TEST_BUFF_SIZE);

            //Act
            sock.SetIPAddr("192.168.1.100");
            sock.SetPort(54321);
            sock.SetType(SERVER);

            //Assert
            Assert::AreEqual(std::string("192.168.1.100"), sock.GetIPAddr());
            Assert::AreEqual(54321, sock.GetPort());
            Assert::AreEqual(SERVER, sock.GetType());
        }

        //Test setter functions during connection
        TEST_METHOD(SetterPostConnectTest)
        {
            //Arrange
            int port = 6000;
            //Create Server
            std::thread serverThread(DummyTCPServer, port);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            //Create Client and Connect
            MySocket sock(CLIENT, "127.0.0.1", port, TCP, TEST_BUFF_SIZE);
            sock.ConnectTCP();

            //Act
            //Attempt to set values while connected
            sock.SetIPAddr("192.168.1.100");
            sock.SetPort(54321);
            sock.SetType(SERVER);

            //Assert
            //Values shouldnt change while connected
            Assert::AreEqual(std::string("127.0.0.1"), sock.GetIPAddr());
            Assert::AreEqual(port, sock.GetPort());
            Assert::AreEqual(CLIENT, sock.GetType());

            sock.DisconnectTCP();
            serverThread.join();
        }


        //Test ConnectTCP when UDP set
        TEST_METHOD(InvalidProtocolTest)
        {
            //Arrange
            MySocket sock(CLIENT, "127.0.0.1", 12345, UDP, TEST_BUFF_SIZE);

            //Act
            sock.ConnectTCP();

            //Assert
            //Since connect fails, setters should still work
            sock.SetIPAddr("10.0.0.1");
            Assert::AreEqual(std::string("10.0.0.1"), sock.GetIPAddr());
        }
        
        //Test sending and recieveing data from dummy server through TCP
        TEST_METHOD(TCPSendAndGetData)
        {
            //Arrange
            int port = 6000;
            //Create Dummy Server Thread
            std::thread serverThread(DummyTCPServer, port);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            //Create Client Socket and connect
            MySocket sock(CLIENT, "127.0.0.1", port, TCP, TEST_BUFF_SIZE);
            sock.ConnectTCP();

            //Act
            //Send message to server
            const char* message = "Hello World!";
            int msgLen = (int) strlen(message);
            sock.SendData(message, msgLen);

            //Allow server to respond
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            //Recieve message from server
            char buffer[TEST_BUFF_SIZE] = { 0 };
            int bytesRecieved = sock.GetData(buffer);

            //Assert
            Assert::AreEqual(0, std::memcmp(buffer, message, strlen(buffer)));

            sock.DisconnectTCP();
            serverThread.join();
        }

        //Test sending and recieveing data from dummy server through UDP
        TEST_METHOD(UDPSendAndGetData)
        {
            //Arrange
            int port = 6000;
            //Create Dummy Server Thread
            std::thread serverThread(DummyUDPServer, port);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            //Create Client Socket
            MySocket sock(CLIENT, "127.0.0.1", port, UDP, TEST_BUFF_SIZE);

            //Act
            //Send message to server
            const char* message = "Hello World!";
            int msgLen = (int) strlen(message);
            sock.SendData(message, msgLen);

            //Allow server to respond
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            //Recieve message from server
            char buffer[TEST_BUFF_SIZE] = { 0 };
            int bytesRecieved = sock.GetData(buffer);

            //Assert
            Assert::AreEqual(0, std::memcmp(buffer, message, strlen(buffer)));

            serverThread.join();
        }


    };
}
