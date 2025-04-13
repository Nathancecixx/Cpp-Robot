#pragma once

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

enum SocketType {CLIENT,SERVER};

enum ConnectionType {TCP,UDP};

const int DEFAULT_SIZE = 1024; //nothing specific in the instructions

class MySocket {
private:
    char* Buffer;              // "to dynamically allocate RAW buffer space for communication activities"
    SOCKET WelcomeSocket;      // tcp server socket
    SOCKET ConnectionSocket;   // for both TCP and UDP; socket used for client/server communication
    struct sockaddr_in SvrAddr; // connection information
    SocketType mySocket;       // the type of socket
    std::string IPAddr;        // IP address
    int Port;                  // port number
    ConnectionType connectionType; //TCP or UDP
    bool bTCPConnect;          // there is a tcp connection or nah
    int MaxSize;               // maximum size for Buffer.



public:
    //C:

    MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);

    //D
    ~MySocket();

    //3 way handshake
    void ConnectTCP();

    //4 way handshake
    void DisconnectTCP();

    //transmit data over the socket, works for both TCP and UDP
    void SendData(const char*, int);

    //receive data from the socket and returns the number of bytes received
    int GetData(char*);

    //returns the IP address
    std::string GetIPAddr();

    //sets the IP address. If a connection is already established than print an error message
    void SetIPAddr(std::string);

    //returns the port number
    int GetPort();

    //sets the port number. If a connection is already established than print an error message
    void SetPort(int);

    //returns the SocketType for this instance
    SocketType GetType();

    //changes the SocketType. and prevent changes if a connection is active
    void SetType(SocketType type);
};
