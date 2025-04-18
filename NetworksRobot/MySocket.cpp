#include "MySocket.h"
#include <iostream>
#include <cstring>

//C: initializes Winsock, the member variables, and creates the sockets
MySocket::MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int bufSize){
    #ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed\n";
        }
    #endif

    //the member variables
    mySocket = type;
    IPAddr = ip;
    Port = port;
    connectionType = connType;
    bTCPConnect = false;
    if (bufSize) {
        MaxSize = bufSize;
    } else {
        MaxSize = DEFAULT_SIZE;}

    //allocate the buffer
    Buffer = new char[MaxSize];
    memset(Buffer, 0, MaxSize);

    //since TCP uses SOCK_STREAM, UDP uses SOCK_DGRAM:
    int sockType;
    if (connectionType == TCP) {
        sockType = SOCK_STREAM;
    }else {
        sockType = SOCK_DGRAM;}


    //create the communication socket
    int protocol;
    if (connectionType == TCP) {
        protocol = IPPROTO_TCP;
    } else {
        protocol = IPPROTO_UDP;}

    ConnectionSocket = socket(AF_INET, sockType, protocol); //make the socket depending on the protocol

    if (ConnectionSocket == INVALID_SOCKET) 
        PRINT_SOCKET_ERROR("Oops, failed to create the connection socket");

    //set up the server address and whatnot
    memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(Port);
    inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);

    //for a tcp server, we need to create and bind a welcome socket
    if (mySocket == SERVER && connectionType == TCP) {
        WelcomeSocket = socket(AF_INET, sockType, IPPROTO_TCP);
        if (WelcomeSocket == INVALID_SOCKET) {
            PRINT_SOCKET_ERROR("Oops, failed to create the welcome socket");
            SOCKET_CLEANUP();
            return;
        }
        if (bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            PRINT_SOCKET_ERROR("Oops, failed to bind socket");
            CLOSESOCKET(WelcomeSocket);
            SOCKET_CLEANUP();
            return;
        }
        if (listen(WelcomeSocket, SOMAXCONN) == SOCKET_ERROR) {
            PRINT_SOCKET_ERROR("Oops, failed to listen");
            CLOSESOCKET(WelcomeSocket);
            SOCKET_CLEANUP();
            return;
        }
    }
}

//D: closes any open sockets and cleans up the allocated memory
MySocket::~MySocket()
{
    if (Buffer) {
        delete[] Buffer;
    }
    if (ConnectionSocket != INVALID_SOCKET) {
        CLOSESOCKET(ConnectionSocket);
    }
    if (mySocket == SERVER && connectionType == TCP && WelcomeSocket != INVALID_SOCKET) {
        CLOSESOCKET(WelcomeSocket);
    }
    SOCKET_CLEANUP();
}

//make a TCP connection for the clients
void MySocket::ConnectTCP()
{
    if (connectionType != TCP || mySocket != CLIENT) {
        std::cerr << "Oops, cannot connect to tcp when using UDP" << std::endl;
        return;
    }
    int result = connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
    if (result == SOCKET_ERROR) {
        PRINT_SOCKET_ERROR("Oops, failed to connect");
        CLOSESOCKET(ConnectionSocket);
        ConnectionSocket = INVALID_SOCKET;
    }
    else {
        bTCPConnect = true;
    }
}

//disconnect a TCP connection
void MySocket::DisconnectTCP()
{
    if (connectionType != TCP || !bTCPConnect)
        return;

    CLOSESOCKET(ConnectionSocket);
    ConnectionSocket = INVALID_SOCKET;
    bTCPConnect = false;
}

//send a block of RAW data, works for both TCP and UDP
void MySocket::SendData(const char* data, int length) {
    if (data == nullptr) return;

    if (connectionType == TCP) {
        int result = send(ConnectionSocket, data, length, 0);
        if (result == SOCKET_ERROR)
            PRINT_SOCKET_ERROR("Oops, tcp send failed");
    }
    else {
        int result = sendto(ConnectionSocket, data, length, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
        if (result == SOCKET_ERROR)
            PRINT_SOCKET_ERROR("Oops, failed to sendto with UDP");
    }
}

//receive data from the socket into the provided memory and return the number of bytes received
int MySocket::GetData(char* dest){
    if (dest == nullptr) return -1;

    int received = 0;
    if (connectionType == TCP) {
        received = recv(ConnectionSocket, this->Buffer, MaxSize, 0);
    } else {
        addr_len addrLen = sizeof(SvrAddr);
        received = recvfrom(ConnectionSocket, this->Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &addrLen);
    }

    if (received) {
        memcpy(dest, this->Buffer, received);
    }
    return received;
}

//returns the IP address
std::string MySocket::GetIPAddr(){
    return IPAddr;
}

//sets the IP address. If a connection is already established than print an error mess
void MySocket::SetIPAddr(std::string ip){
    if (bTCPConnect) {
        std::cerr << "Oops, cannot change IP because a connection is already established." << std::endl;
        return;
    }
    IPAddr = ip;
    inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);
}

//returns the port number
int MySocket::GetPort(){
    return Port;
}

//sets the port number.If a connection is already established than print an error message
void MySocket::SetPort(int port){
    if (bTCPConnect) {
        std::cerr << "Oops, cannot change port because a  connection is already established." << std::endl;
        return;
    }
    Port = port;
    SvrAddr.sin_port = htons(Port);
}

//returns the SocketType for this instance
SocketType MySocket::GetType(){
    return mySocket;
}

//changes the SocketType. and prevent changes if a connection is active
void MySocket::SetType(SocketType type)
{
    if (bTCPConnect || (mySocket == SERVER && connectionType == TCP)) {
        std::cerr << "Oops, cannot change socket type when connection is active." << std::endl;
        return;
    }
    mySocket = type;
}
