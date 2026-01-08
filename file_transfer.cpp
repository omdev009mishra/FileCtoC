#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

#define PORT 8888
#define BUFFER_SIZE 4096

using namespace std;

class FileTransfer {
private:
    WSADATA wsaData;
    
public:
    FileTransfer() {
        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "WSAStartup failed!" << endl;
            exit(1);
        }
    }
    
    ~FileTransfer() {
        WSACleanup();
    }
    
    // Server mode - Receives file
    void receiveFile(const string& saveAs) {
        SOCKET serverSocket, clientSocket;
        struct sockaddr_in serverAddr, clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        
        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            cerr << "Socket creation failed! Error: " << WSAGetLastError() << endl;
            return;
        }
        
        // Prepare server address
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);
        
        // Bind socket
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Bind failed! Error: " << WSAGetLastError() << endl;
            closesocket(serverSocket);
            return;
        }
        
        // Listen for connections
        if (listen(serverSocket, 1) == SOCKET_ERROR) {
            cerr << "Listen failed! Error: " << WSAGetLastError() << endl;
            closesocket(serverSocket);
            return;
        }
        
        cout << "Server listening on port " << PORT << "..." << endl;
        cout << "Waiting for incoming connection..." << endl;
        
        // Accept connection
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed! Error: " << WSAGetLastError() << endl;
            closesocket(serverSocket);
            return;
        }
        
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        cout << "Connection established with " << clientIP << endl;
        
        // Receive filename length
        int filenameLen;
        recv(clientSocket, (char*)&filenameLen, sizeof(filenameLen), 0);
        
        // Receive filename
        char* filename = new char[filenameLen + 1];
        recv(clientSocket, filename, filenameLen, 0);
        filename[filenameLen] = '\0';
        
        cout << "Receiving file: " << filename << endl;
        
        // Receive file size
        long long fileSize;
        recv(clientSocket, (char*)&fileSize, sizeof(fileSize), 0);
        
        cout << "File size: " << fileSize << " bytes" << endl;
        
        // Open file for writing
        string outputFile = saveAs.empty() ? filename : saveAs;
        ofstream file(outputFile, ios::binary);
        
        if (!file.is_open()) {
            cerr << "Failed to create file!" << endl;
            delete[] filename;
            closesocket(clientSocket);
            closesocket(serverSocket);
            return;
        }
        
        // Receive file data
        char buffer[BUFFER_SIZE];
        long long totalReceived = 0;
        int bytesReceived;
        
        while (totalReceived < fileSize) {
            bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesReceived <= 0) break;
            
            file.write(buffer, bytesReceived);
            totalReceived += bytesReceived;
            
            // Show progress
            int progress = (int)((totalReceived * 100) / fileSize);
            cout << "\rProgress: " << progress << "% [" << totalReceived << "/" << fileSize << " bytes]" << flush;
        }
        
        cout << endl << "File received successfully and saved as: " << outputFile << endl;
        
        file.close();
        delete[] filename;
        closesocket(clientSocket);
        closesocket(serverSocket);
    }
    
    // Client mode - Sends file
    void sendFile(const string& filename, const string& serverIP) {
        SOCKET clientSocket;
        struct sockaddr_in serverAddr;
        
        // Create socket
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Socket creation failed! Error: " << WSAGetLastError() << endl;
            return;
        }
        
        // Prepare server address
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        
        if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
            cerr << "Invalid address!" << endl;
            closesocket(clientSocket);
            return;
        }
        
        cout << "Connecting to " << serverIP << ":" << PORT << "..." << endl;
        
        // Connect to server
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Connection failed! Error: " << WSAGetLastError() << endl;
            closesocket(clientSocket);
            return;
        }
        
        cout << "Connected successfully!" << endl;
        
        // Open file
        ifstream file(filename, ios::binary | ios::ate);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << filename << endl;
            closesocket(clientSocket);
            return;
        }
        
        // Get file size
        long long fileSize = file.tellg();
        file.seekg(0, ios::beg);
        
        // Extract just the filename (without path)
        string baseFilename = filename;
        size_t pos = baseFilename.find_last_of("\\/");
        if (pos != string::npos) {
            baseFilename = baseFilename.substr(pos + 1);
        }
        
        // Send filename length
        int filenameLen = baseFilename.length();
        send(clientSocket, (char*)&filenameLen, sizeof(filenameLen), 0);
        
        // Send filename
        send(clientSocket, baseFilename.c_str(), filenameLen, 0);
        
        // Send file size
        send(clientSocket, (char*)&fileSize, sizeof(fileSize), 0);
        
        cout << "Sending file: " << baseFilename << " (" << fileSize << " bytes)" << endl;
        
        // Send file data
        char buffer[BUFFER_SIZE];
        long long totalSent = 0;
        
        while (!file.eof() && totalSent < fileSize) {
            file.read(buffer, BUFFER_SIZE);
            int bytesRead = file.gcount();
            
            int bytesSent = send(clientSocket, buffer, bytesRead, 0);
            if (bytesSent <= 0) break;
            
            totalSent += bytesSent;
            
            // Show progress
            int progress = (int)((totalSent * 100) / fileSize);
            cout << "\rProgress: " << progress << "% [" << totalSent << "/" << fileSize << " bytes]" << flush;
        }
        
        cout << endl << "File sent successfully!" << endl;
        
        file.close();
        closesocket(clientSocket);
    }
    
    // Display local IP addresses with connection type detection
    void showIPAddresses() {
        cout << "\n========================================" << endl;
        cout << "  Network Connections & IP Addresses" << endl;
        cout << "========================================" << endl;
        
        ULONG bufferSize = 0;
        GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &bufferSize);
        
        PIP_ADAPTER_ADDRESSES adapterAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
        
        if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &bufferSize) == NO_ERROR) {
            PIP_ADAPTER_ADDRESSES adapter = adapterAddresses;
            int count = 1;
            
            while (adapter) {
                if (adapter->OperStatus == IfOperStatusUp) {
                    // Convert wide string to regular string
                    char friendlyName[256];
                    wcstombs_s(NULL, friendlyName, 256, adapter->FriendlyName, 255);
                    
                    // Get connection type
                    string connType;
                    if (adapter->IfType == IF_TYPE_ETHERNET_CSMACD) {
                        connType = "Ethernet";
                    } else if (adapter->IfType == IF_TYPE_USB) {
                        connType = "USB";
                    } else if (adapter->IfType == IF_TYPE_IEEE80211) {
                        connType = "WiFi";
                    } else {
                        connType = "Other";
                    }
                    
                    // Check if USB in name
                    string name(friendlyName);
                    if (name.find("USB") != string::npos || name.find("RNDIS") != string::npos) {
                        connType = "USB Network";
                    }
                    
                    // Get IP address
                    PIP_ADAPTER_UNICAST_ADDRESS unicast = adapter->FirstUnicastAddress;
                    if (unicast) {
                        struct sockaddr_in* addr = (struct sockaddr_in*)unicast->Address.lpSockaddr;
                        char ipStr[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &addr->sin_addr, ipStr, INET_ADDRSTRLEN);
                        
                        if (string(ipStr) != "0.0.0.0") {
                            cout << "\n[" << count++ << "] " << friendlyName << endl;
                            cout << "    Type: " << connType << endl;
                            cout << "    IP Address: " << ipStr << endl;
                            cout << "    Status: Connected" << endl;
                        }
                    }
                }
                adapter = adapter->Next;
            }
        }
        
        free(adapterAddresses);
        
        cout << "\n========================================" << endl;
        cout << "\nSupported Connections:" << endl;
        cout << "  • Ethernet cable (RJ45)" << endl;
        cout << "  • USB-C to USB-C (with network mode)" << endl;
        cout << "  • USB to Ethernet adapters" << endl;
        cout << "  • USB Direct Link cables" << endl;
        cout << "\nFor USB setup: See README.md" << endl;
        cout << "========================================\n" << endl;
    }
};

void printUsage() {
    cout << "\n===========================================" << endl;
    cout << "  PC-to-PC File Transfer Program" << endl;
    cout << "  Supports: Ethernet & USB Connections" << endl;
    cout << "===========================================" << endl;
    cout << "\nUsage:" << endl;
    cout << "  Server Mode (Receive): file_transfer.exe -r [output_filename]" << endl;
    cout << "  Client Mode (Send):    file_transfer.exe -s <file_to_send> <server_ip>" << endl;
    cout << "  Show Connections:      file_transfer.exe -ip" << endl;
    cout << "\nExamples:" << endl;
    cout << "  file_transfer.exe -ip" << endl;
    cout << "  file_transfer.exe -r received_file.txt" << endl;
    cout << "  file_transfer.exe -s myfile.pdf 192.168.1.100" << endl;
    cout << "\nSupported Cables:" << endl;
    cout << "  ✓ Ethernet cable (RJ45)" << endl;
    cout << "  ✓ USB-C to USB-C (with Mobile Hotspot)" << endl;
    cout << "  ✓ USB-C/A to Ethernet adapters" << endl;
    cout << "  ✓ USB Direct Link cable" << endl;
    cout << "\nSetup Help: See README.md for USB setup instructions" << endl;
    cout << "===========================================" << endl;
}

int main(int argc, char* argv[]) {
    FileTransfer ft;
    
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    string mode = argv[1];
    
    if (mode == "-ip") {
        // Show IP addresses
        ft.showIPAddresses();
    }
    else if (mode == "-r") {
        // Server mode - Receive file
        string outputFile = (argc > 2) ? argv[2] : "";
        ft.receiveFile(outputFile);
    }
    else if (mode == "-s") {
        // Client mode - Send file
        if (argc < 4) {
            cerr << "Error: Missing arguments for send mode!" << endl;
            printUsage();
            return 1;
        }
        string filename = argv[2];
        string serverIP = argv[3];
        ft.sendFile(filename, serverIP);
    }
    else {
        cout << "Invalid option!" << endl;
        printUsage();
        return 1;
    }
    
    return 0;
}
