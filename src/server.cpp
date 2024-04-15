//Shane Tucker - CS370
//Server application for basic ftp server
//IMPORTANT: when compiling with g++, include '-lwsock32'

#include <iostream>
#include <string>
# include <winsock2.h>

#define DEFAULT_PORT 8080
#define buffer_size 1024

SOCKET serverSetUp();

using namespace std;

int main() {
    string hostname;
    int sByteCount, rByteCount;
    char sendBuffer[buffer_size], receiveBuffer[buffer_size];

    cout << "Enter hostname: "; //Get hostname from user
    getline(cin, hostname);
    
    SOCKET serverSocket = serverSetUp(); //Establish Connection

    strcpy(sendBuffer, hostname.c_str());
    sByteCount = send(serverSocket, sendBuffer, buffer_size, 0);
    if (sByteCount == SOCKET_ERROR) {
        cout << "Send error." << endl;
    }
}

SOCKET serverSetUp() {//Major help from: https://medium.com/@tharunappu2004/creating-a-simple-tcp-server-in-c-using-winsock-b75dde86dd39
    WSADATA wsaData; //Initialize Winsock
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) {
        cout << "Error: Winsock dll not found." << endl;
        exit(-1);
    }
    
    SOCKET serverSocket;
    serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) { // Check for socket creation success
        cout << "Error creating socket." << endl;
        WSACleanup();
        exit(-1);
    }
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY; //Allows any IP to connect
    service.sin_port = htons(DEFAULT_PORT);
    if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&service), sizeof(service)) == SOCKET_ERROR) { //Bind socket and check for error
        cout << "Bind failed. " << endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(-1);
    }
    if (listen(serverSocket, 1) == SOCKET_ERROR) { //Start listen and check for error
        cout << "Listen error. " << std::endl;
    }
    SOCKET acceptSocket;
    acceptSocket = accept(serverSocket, nullptr, nullptr); //Accept connection
    if (acceptSocket == INVALID_SOCKET) { //Checks if invalid accept
        cout << "Error accepting socket. " << endl;
        WSACleanup();
        exit(-1);
    } //Program can now send and receive data

    return acceptSocket;
}