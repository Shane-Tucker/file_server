//Shane Tucker - CS370
//Client application for basic ftp server
//IMPORTANT: when compiling with g++, include '-lwsock32'

#include <fstream>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <string>
#include <winsock2.h>

#define DEFAULT_PORT 8080
#define buffer_size 1024


SOCKET clientSetUp();

using namespace std;

int main() {
    string userCommand, currentUser, hostname;
    bool loggedIn = false; 
    int sByteCount, rByteCount;
    char sendBuffer[buffer_size], receiveBuffer[buffer_size];

    cout << "Enter hostname: "; //Get hostname from user
    getline(cin, hostname);

    SOCKET clientSocket = clientSetUp(); //Establish Connection
    rByteCount = recv(clientSocket, receiveBuffer, buffer_size, 0);

    string message(receiveBuffer);
    cout << "Connected to server: " << message << endl;

    cout << "Enter 'help' for list of commands" << endl;
    while (true) {
        cout << "$ ";
        cin >> userCommand;

        if (userCommand == "help") { //Gives user list of commands
            cout << "login [user]\t\t\tAllows the user to sign in through the client" << endl <<
                    "logout\t\t\tLogs the user out of current account" << endl <<
                    "ls\t\t\tDisplays current directory on client" << endl <<
                    "cd [directory]\t\tChanges client directory" << endl <<
                    "ls_s\t\t\tDisplays current directory in server" << endl <<
                    "upload [filename]\tUploads [filename] to server from client" << endl <<
                    "download [filename]\tDownloads [filename] from server to client" << endl <<
                    "exit\t\t\tCloses client program" << endl <<
                    "shutdown\t\tCloses client and server program" << endl;
        }else if (userCommand == "login") { //Logs user in to account
            if (loggedIn == false) {
                cin >> currentUser;
                cout << "Logged in as user: " << currentUser << endl;
                loggedIn = true;
            } else {
                cout << "Already logged in as user: " << currentUser << endl;
            }
            
        }else if (userCommand == "logout") { //Log user out of current account
            if (loggedIn == false) {
                cout << "Not logged into any account" << endl;
            } else {
                cout << "Logged out of user: " << currentUser << endl;
                currentUser = "";
                loggedIn = false;
            }
        }else if (userCommand == "ls") {
        
        }else if (userCommand == "cd") {
            
        }else if (userCommand == "ls_s") {
            
        }else if (userCommand == "upload") {
            
        }else if (userCommand == "download") {
            
        }else if (userCommand == "exit") { //Just close 
            return 0;
        }else if (userCommand == "shutdown") {
            
        }else if (loggedIn == false) {
            cout << "Must login before any commands" << endl;
        }else {
            cout << "Invalid Input" << endl;
        }
        cin.clear(); 
        fflush(stdin); //Clears out stdin
    }
}

SOCKET clientSetUp() {//Major help from: https://medium.com/@tharunappu2004/creating-a-simple-tcp-server-in-c-using-winsock-b75dde86dd39
    WSADATA wsaData; //Initialize WSA variables
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) {
        cout << "Error: Winsock dll not found." << endl;
        exit(-1);
    }

    SOCKET clientSocket;
    clientSocket = INVALID_SOCKET;
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) { // Check for socket creation success
        cout << "Error creating socket." << endl;
        WSACleanup();
        exit(-1);
    }
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(INADDR_ANY);  // Replace with the server's IP address
    clientService.sin_port = htons(DEFAULT_PORT);
    if (connect(clientSocket, reinterpret_cast<SOCKADDR*>(&clientService), sizeof(clientService)) == SOCKET_ERROR) { //Connect and check for errors
        cout << "Failed to connect" << endl;
        WSACleanup();
        //exit(-1);
    }

    return clientSocket;
}