//Shane Tucker - CS370
//Server application for basic ftp server
//IMPORTANT: when compiling with g++, include '-lwsock32'

#include <iostream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <winsock2.h>

#define DEFAULT_PORT 8080
#define buffer_size 1024

using namespace std;

SOCKET serverSetUp();
string charArrayToString(char* charArray);
void sendData(string data, SOCKET serverSocket);
string receiveData(SOCKET serverSocket);

int main() {
    WSACleanup(); //Removes any previous connections
    string hostname, message, currentUser, userDirectory, temp;
    bool cont = true;
    std::filesystem::path currentDirectory = std::filesystem::absolute(".files");
    if (!exists(currentDirectory)) { //Creates directory if it doesn't exist already
        create_directory(currentDirectory);
    }

    cout << "Enter hostname: "; //Get hostname from user
    getline(cin, hostname);
    while (cont == true) { //Allows server to connect to a different client once one quits
        SOCKET serverSocket = serverSetUp(); //Establish Connection

        currentDirectory = std::filesystem::absolute(".files"); //Reset variables
        currentUser = "";

        sendData(hostname, serverSocket);

        message = receiveData(serverSocket); //Receive client hostname
        cout << "Connected to client: " << message << endl;

        while (true) { //Run until break statement
            message = receiveData(serverSocket);
            if (WSAGetLastError() == 10054) {
                cout << "Terminating current connection" << endl;
                WSACleanup();
                break;
            } 
            if (message.substr(0, 5) == "login") {
                currentUser = message.substr(6, message.find(" ", 6) - 6);
                userDirectory = "." + currentUser;
                currentDirectory = currentDirectory / userDirectory;
                if (!exists(currentDirectory)) { //Creates directory if it doesn't exist already
                    create_directory(currentDirectory);
                }
                cout << "Logged in as: " << currentUser << " with directory: " << currentDirectory.u8string() << endl;
            } else if (message.substr(0, 6) == "logout") {
                currentUser = "";
                currentDirectory = std::filesystem::absolute(".files");
            } else if (message.substr(0, 4) == "ls_s") { //Help from: https://www.geeksforgeeks.org/file-system-library-in-cpp-17/
                if (exists(currentDirectory) && is_directory(currentDirectory)) {
                    for (const auto& entry : std::filesystem::directory_iterator(currentDirectory)) { //For all entries in directory, change path to string and send to client
                        string stringPath(entry.path().u8string());
                        sendData(stringPath, serverSocket);
                    }
                }
                sendData("STOP!@#$", serverSocket);
                cout << "Displaying: " << currentDirectory.u8string() << endl;
            } else if (message.substr(0, 6) == "upload") {
                ofstream outfile;
                outfile.open(currentDirectory / message.substr(7, message.length() - 7));
                cout << "Uploading: " << message.substr(7, message.length() - 7) << endl;
                while (message != "STOP!@#$") {
                    message = receiveData(serverSocket);
                    if (message != "STOP!@#$") {
                        outfile << message << endl;
                    }
                }
                outfile.close();
            } else if (message.substr(0, 8) == "download") {
                if (!exists(currentDirectory / message.substr(9, message.length() - 9))) { //If file DNE
                    temp = "FILE DNE";
                    sendData(temp, serverSocket);
                } else {
                    temp = "FILE EXISTS";
                    sendData(temp, serverSocket);
                    ifstream readfile;
                    readfile.open(currentDirectory / message.substr(9, message.length() - 9)); //Open file
                    if (!readfile) {
                        cout << "File unable to open" << endl;
                        break;
                    }
                    readfile.seekg(0);
                    while (!readfile.eof()) { //Take line as input, send to client, repeat til eof
                        getline(readfile, message);
                        sendData(message, serverSocket);
                    }
                    sendData("STOP!@#$", serverSocket); //Let client know to stop looking for input
                    readfile.close();
                }
            } else if (message.substr(0, 4) == "exit") {
                WSACleanup();
                cout << "Terminating current connection" << endl;
                break;
            } else if (message.substr(0, 8) == "shutdown") {
                sendData("SHUTDOWN", serverSocket);
                WSACleanup();
                cout << "Shutting down" << endl;
                return 0;
            }
        }
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
        cout << "Error creating socket: " << WSAGetLastError()  << endl;
        WSACleanup();
        exit(-1);
    }
    const char optval = '1';
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == SOCKET_ERROR) {
        cout << "setsockopt fail: " << WSAGetLastError() << endl;
    }
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY; //Allows any IP to connect
    service.sin_port = htons(DEFAULT_PORT);
    if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&service), sizeof(service)) == SOCKET_ERROR) { //Bind socket and check for error
        cout << "Bind failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(-1);
    }
    cout << "Awaiting connection..." << endl;
    if (listen(serverSocket, 1) == SOCKET_ERROR) { //Start listen and check for error
        cout << "Listen error: " << WSAGetLastError()  << endl;
    }
    SOCKET acceptSocket;
    acceptSocket = accept(serverSocket, nullptr, nullptr); //Accept connection
    if (acceptSocket == INVALID_SOCKET) { //Checks if invalid accept
        cout << "Error accepting socket" << WSAGetLastError()  << endl;
        WSACleanup();
        exit(-1);
    } //Program can now send and receive data

    return acceptSocket;
}

string charArrayToString(char* charArray) {
    string s;
    for (int i = 0; i < sizeof(charArray); i++) {
        s = s + charArray[i];
    }
    return s;
}

void sendData(string data, SOCKET serverSocket) {
    const char* dataPtr = data.data(); 
    size_t dataSize = data.size();
    int bytesSent, bytesSentPre; 
    while (dataSize > 0) {
        string strDataSize = to_string(dataSize); 
        bytesSentPre = send(serverSocket, strDataSize.c_str(), 1024, 0);
        if (bytesSentPre == SOCKET_ERROR) {
            cout << "Send error: " << WSAGetLastError << endl;
            break;
        }
        bytesSent = send(serverSocket, dataPtr, dataSize, 0); //Send message to client
        if (bytesSent == SOCKET_ERROR) {
            cout << "Send error: " << WSAGetLastError() << endl;
            break;
        }
        dataPtr += bytesSent;
        dataSize -= bytesSent;
    }
    bytesSent = send(serverSocket, "STOP*&^", buffer_size, 0);
    if (bytesSent == SOCKET_ERROR) {
        cout << "Send error: " << WSAGetLastError() << endl;
    }
}

string receiveData(SOCKET serverSocket) {
    const char* dataPtr;
    size_t dataSize = 1;
    string temp, data = "";
    int bytesRec;
    char receiveBuffer[buffer_size];
    while (dataSize > 0) {
        bytesRec = recv(serverSocket, receiveBuffer, sizeof(receiveBuffer), 0); //Receive size of data being sent in next socket
        if (bytesRec == SOCKET_ERROR) {
            cout << "Read error: " << WSAGetLastError() << endl;
            break;
        }
        temp = charArrayToString(receiveBuffer);
        stringstream sstream(temp);
        sstream >> dataSize;
        char* receiveDyn = new char[dataSize];
        bytesRec = recv(serverSocket, receiveBuffer, dataSize, 0); //Receive socket with previously defined size
        if (bytesRec == SOCKET_ERROR) {
            cout << "Read error: " << WSAGetLastError() << endl;
            break;
        }
        for(int i = 0; i < dataSize; i++) {
            data = data + receiveBuffer[i];
        }
        if (bytesRec == dataSize) {
            break;
        }
    }

    return data;
}