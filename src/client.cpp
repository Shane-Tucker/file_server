//Shane Tucker - CS370
//Client application for basic ftp server
//IMPORTANT: when compiling with g++, include '-lwsock32'

#include <fstream>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <winsock2.h>

#define DEFAULT_PORT 8080
#define buffer_size 1024

using namespace std;

SOCKET clientSetUp();
string charArrayToString(char* charArray);
void sendData(string data, SOCKET clientSocket);
string receiveData(SOCKET clientSocket);

int main() {
    WSACleanup(); //Removes any previous connections
    string userCommand, currentUser, hostname, message, temp, filename;
    bool loggedIn = false; 
    std::filesystem::path currentDirectory = ".";
    currentDirectory = absolute(currentDirectory);

    cout << "Enter hostname: "; //Get hostname from user
    getline(cin, hostname);

    SOCKET clientSocket = clientSetUp(); //Establish Connection

    message = receiveData(clientSocket); //Receive server hostname
    cout << "Connected to server: " << message << endl;
    
    sendData(hostname, clientSocket);

    cout << "Enter 'help' for list of commands" << endl;
    while (true) {
        cout << "$ ";
        getline(cin, userCommand);

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
        } else if (userCommand.substr(0, 5) == "login") { //Logs user in to account
            if (loggedIn == false) {
                if (sizeof(userCommand) > 6) {
                    currentUser = userCommand.substr(6, userCommand.find(" ", 6) - 6);
                }
                cout << "Logged in as user: " << currentUser << endl;
                loggedIn = true;
                sendData(userCommand, clientSocket);
            } else {
                cout << "Already logged in as user: " << currentUser << endl;
            }
            
        } else if (userCommand == "logout") { //Log user out of current account
            if (loggedIn == false) {
                cout << "Not logged into any account" << endl;
            } else {
                cout << "Logged out of user: " << currentUser << endl;
                sendData(userCommand, clientSocket); 
                currentUser = "";
                loggedIn = false;
            }
        } else if (userCommand.substr(0, 4) == "ls_s" && loggedIn == true) { //If user uses ls_s
            sendData(userCommand, clientSocket); //Send command to server
            message = "";
            while (message != "STOP!@#$") {
                message = receiveData(clientSocket);
                if (message != "STOP!@#$") {
                    cout << message << endl;
                }
            }
        } else if (userCommand.substr(0, 2) == "ls" && loggedIn == true && userCommand.substr(0, 4) != "ls_s") { //Help from: https://www.geeksforgeeks.org/file-system-library-in-cpp-17/
            if (exists(currentDirectory) && is_directory(currentDirectory)) {
                for (const auto& entry : std::filesystem::directory_iterator(currentDirectory)) {
                    string string_path(entry.path().u8string());
                    cout << string_path << endl;
                }
            } else {
                cout << "Directory not found" << endl;
            }
        } else if (userCommand.substr(0, 2) == "cd" && loggedIn == true) {
            if (userCommand.length() <= 3) {
                cout << "Invalid Input" << endl;
            } else if (userCommand.substr(3, userCommand.length() - 3) != "..") { //Finds rest of string
                if (exists(currentDirectory / userCommand.substr(3, userCommand.length() - 3))) { //Checks if directory exists
                    currentDirectory = currentDirectory / userCommand.substr(3, userCommand.length() - 3); //Assigns as new directory
                }
            } else if (userCommand.substr(3, userCommand.length() - 3) == "..") { 
                temp = currentDirectory.u8string();
                for (int i = temp.length(); i >= 0; i--) {
                    if (temp[i] == '\\') {
                        temp.erase(i, temp.length() - i);
                        currentDirectory = temp;
                        break;
                    }
                }
            } else {
                cout << "Directory not found" << endl;
            }   
        } else if (userCommand.substr(0, 6) == "upload" && loggedIn == true) {
            if (userCommand.length() <= 7) {
                cout << "Filename needed" << endl;
            } else if (exists(currentDirectory / userCommand.substr(7, userCommand.length() - 7))) {
                sendData(userCommand, clientSocket);
                ifstream readfile;
                readfile.open(currentDirectory / userCommand.substr(7, userCommand.length() - 7)); //Open file
                if (!readfile) {
                    cout << "File unable to open" << endl;
                    break;
                }
                while (!readfile.eof()) { //Take line as input, send to server, repeat til end of file
                    getline(readfile, message);
                    sendData(message, clientSocket);
                }
                sendData("STOP!@#$", clientSocket); //Send stop message so server knows to stop. While possible for this to be in a file, highly unlikely
                readfile.close(); //Close file
            } else {
                cout << "File not found" << endl;
            }
        } else if (userCommand.substr(0, 8) == "download" && loggedIn == true) {
            sendData(userCommand, clientSocket);
            if (userCommand.length() <= 9) { //If no filename
                cout << "Filename needed" << endl; 
            } else if (receiveData(clientSocket) == "FILE DNE") { //If file DNE on server
                cout << "File does not exist" << endl;
            } else {
                ofstream outfile; 
                outfile.open(currentDirectory / userCommand.substr(9, userCommand.length() - 9)); //Open file
                cout << "Downloading: " << userCommand.substr(9, userCommand.length() - 9) << endl;
                while (message != "STOP!@#$") {
                    message = receiveData(clientSocket);
                    if (message != "STOP!@#$") {
                        outfile << message << endl;
                    }
                }
                outfile.close();
            }
        } else if (userCommand.substr(0, 4) == "exit") { //Just exit client
            sendData(userCommand, clientSocket);
            WSACleanup();
            return 0;
        } else if (userCommand.substr(0, 8) == "shutdown") { //Shutsdown both server and client
            sendData(userCommand, clientSocket);
            WSACleanup();
            return 0;
        } else if (loggedIn == false) {
            cout << "Must login before any commands" << endl;
        } else {
            cout << "Invalid Input" << endl;
        }
        cin.clear(); 
        fflush(stdin); //Clears out stdin
    }
}

SOCKET clientSetUp() {//Help from: https://medium.com/@tharunappu2004/creating-a-simple-tcp-server-in-c-using-winsock-b75dde86dd39
    string serverIP;
    cout << "Enter IP to connect to: ";
    cin >> serverIP;
    
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
    const char optval = '1';
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == SOCKET_ERROR) {
        cout << "setsockopt fail: " << WSAGetLastError() << endl;
    }
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(serverIP.c_str());  // Replace with the server's IP address
    clientService.sin_port = htons(DEFAULT_PORT);
    if (connect(clientSocket, reinterpret_cast<SOCKADDR*>(&clientService), sizeof(clientService)) == SOCKET_ERROR) { //Connect and check for errors
        cout << "Failed to connect" << endl;
        WSACleanup();
        exit(-1);
    }

    return clientSocket;
}

string charArrayToString(char* charArray) {
    string s = "";
    for (int i = 0; i < sizeof(charArray); i++) {
        s = s + charArray[i];
    }
    return s;
}

void sendData(string data, SOCKET clientSocket) {
    const char* dataPtr = data.data();
    size_t dataSize = data.size();
    int bytesSent, bytesSentPre; 
    while (dataSize > 0) {
        string strDataSize = to_string(dataSize);
        bytesSentPre = send(clientSocket, strDataSize.c_str(), buffer_size, 0); //Send size of string to be sent
        if (bytesSentPre == SOCKET_ERROR) {
            cout << "Send error: " << WSAGetLastError() << endl;
        }
        bytesSent = send(clientSocket, dataPtr, dataSize, 0);
        if (bytesSent == SOCKET_ERROR) {
            cout << "Send error: " << WSAGetLastError() << endl;
            break;
        }
        dataPtr += bytesSent;
        dataSize -= bytesSent;
    }
}

string receiveData(SOCKET clientSocket) {
    const char* dataPtr; 
    size_t dataSize = 1;
    string temp, data = "";
    int bytesRec;
    char receiveBuffer[buffer_size];
    while (dataSize > 0) {
        bytesRec = recv(clientSocket, receiveBuffer, sizeof(receiveBuffer), 0);
        if (bytesRec == SOCKET_ERROR) {
            cout << "Read error: " << WSAGetLastError() << endl;
            break;
        }
        temp = charArrayToString(receiveBuffer);
        stringstream sstream(temp);
        sstream >> dataSize;
        bytesRec = recv(clientSocket, receiveBuffer, dataSize, 0);
        if (bytesRec == SOCKET_ERROR) {
            cout << "Read error: " << WSAGetLastError() << endl;
            break;
        }
        for (int i = 0; i < dataSize; i++) {
            data = data + receiveBuffer[i];
        }
        if (bytesRec == dataSize) {
            break;
        }
    }
    return data;
}