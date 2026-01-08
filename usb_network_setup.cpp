#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct NetworkAdapter {
    string name;
    string description;
    string ipAddress;
    string type;
    bool isConnected;
};

class USBNetworkHelper {
public:
    // Detect all network adapters
    static vector<NetworkAdapter> getNetworkAdapters() {
        vector<NetworkAdapter> adapters;
        
        ULONG bufferSize = 0;
        GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &bufferSize);
        
        PIP_ADAPTER_ADDRESSES adapterAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
        
        if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &bufferSize) == NO_ERROR) {
            PIP_ADAPTER_ADDRESSES adapter = adapterAddresses;
            
            while (adapter) {
                NetworkAdapter netAdapter;
                
                // Convert wide string to regular string
                wcstombs_s(NULL, NULL, 0, adapter->FriendlyName, 0);
                char friendlyName[256];
                wcstombs_s(NULL, friendlyName, 256, adapter->FriendlyName, 255);
                netAdapter.name = friendlyName;
                
                netAdapter.description = adapter->AdapterName;
                
                // Get IP address
                PIP_ADAPTER_UNICAST_ADDRESS unicast = adapter->FirstUnicastAddress;
                if (unicast) {
                    struct sockaddr_in* addr = (struct sockaddr_in*)unicast->Address.lpSockaddr;
                    char ipStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &addr->sin_addr, ipStr, INET_ADDRSTRLEN);
                    netAdapter.ipAddress = ipStr;
                }
                
                // Determine adapter type
                if (adapter->IfType == IF_TYPE_ETHERNET_CSMACD) {
                    netAdapter.type = "Ethernet";
                } else if (adapter->IfType == IF_TYPE_USB) {
                    netAdapter.type = "USB";
                } else if (adapter->IfType == IF_TYPE_IEEE80211) {
                    netAdapter.type = "WiFi";
                } else {
                    netAdapter.type = "Other";
                }
                
                netAdapter.isConnected = (adapter->OperStatus == IfOperStatusUp);
                
                adapters.push_back(netAdapter);
                adapter = adapter->Next;
            }
        }
        
        free(adapterAddresses);
        return adapters;
    }
    
    // Display all available network connections
    static void displayConnections() {
        cout << "\n========================================" << endl;
        cout << "  Available Network Connections" << endl;
        cout << "========================================" << endl;
        
        vector<NetworkAdapter> adapters = getNetworkAdapters();
        int count = 1;
        
        for (const auto& adapter : adapters) {
            if (!adapter.isConnected && adapter.ipAddress.empty()) continue;
            
            cout << "\n[" << count++ << "] " << adapter.name << endl;
            cout << "    Type: " << adapter.type << endl;
            cout << "    Status: " << (adapter.isConnected ? "Connected" : "Disconnected") << endl;
            if (!adapter.ipAddress.empty()) {
                cout << "    IP Address: " << adapter.ipAddress << endl;
            }
        }
        
        cout << "\n========================================" << endl;
    }
    
    // Check if USB network is available
    static bool isUSBNetworkAvailable() {
        vector<NetworkAdapter> adapters = getNetworkAdapters();
        
        for (const auto& adapter : adapters) {
            if (adapter.type == "USB" && adapter.isConnected && !adapter.ipAddress.empty()) {
                return true;
            }
            // Check for USB-based Ethernet adapters
            if (adapter.name.find("USB") != string::npos && 
                adapter.isConnected && 
                !adapter.ipAddress.empty()) {
                return true;
            }
        }
        
        return false;
    }
    
    // Get recommended IP for connection
    static string getRecommendedIP() {
        vector<NetworkAdapter> adapters = getNetworkAdapters();
        
        // First, look for USB connections
        for (const auto& adapter : adapters) {
            if ((adapter.type == "USB" || adapter.name.find("USB") != string::npos) && 
                adapter.isConnected && 
                !adapter.ipAddress.empty() &&
                adapter.ipAddress != "0.0.0.0") {
                return adapter.ipAddress;
            }
        }
        
        // Then Ethernet
        for (const auto& adapter : adapters) {
            if (adapter.type == "Ethernet" && 
                adapter.isConnected && 
                !adapter.ipAddress.empty() &&
                adapter.ipAddress != "0.0.0.0") {
                return adapter.ipAddress;
            }
        }
        
        return "Not Found";
    }
    
    // Print setup instructions for USB networking
    static void printUSBSetupGuide() {
        cout << "\n========================================" << endl;
        cout << "  USB Cable Connection Setup Guide" << endl;
        cout << "========================================" << endl;
        cout << "\nMethod 1: USB-C to USB-C Direct (Windows 10/11)" << endl;
        cout << "--------------------------------------------" << endl;
        cout << "1. Connect both PCs with USB-C cable" << endl;
        cout << "2. On PC 1: Go to Settings > Network & Internet" << endl;
        cout << "3. Click 'Mobile hotspot' and enable it" << endl;
        cout << "4. Choose 'Share over: Bluetooth or USB'" << endl;
        cout << "5. On PC 2: The USB network should appear automatically" << endl;
        cout << "6. Both PCs will get IPs in the 192.168.137.x range" << endl;
        
        cout << "\n\nMethod 2: USB-C/A to Ethernet Adapters" << endl;
        cout << "--------------------------------------------" << endl;
        cout << "1. Connect USB-to-Ethernet adapters to both PCs" << endl;
        cout << "2. Connect adapters with an Ethernet cable" << endl;
        cout << "3. Windows will detect them as Ethernet connections" << endl;
        cout << "4. Set static IPs:" << endl;
        cout << "   PC 1: 192.168.1.1, Subnet: 255.255.255.0" << endl;
        cout << "   PC 2: 192.168.1.2, Subnet: 255.255.255.0" << endl;
        
        cout << "\n\nMethod 3: Standard Ethernet Cable" << endl;
        cout << "--------------------------------------------" << endl;
        cout << "1. Connect both PCs with Ethernet/RJ45 cable" << endl;
        cout << "2. Modern PCs auto-detect (no crossover needed)" << endl;
        cout << "3. Set static IPs or use auto-configuration" << endl;
        
        cout << "\n\nMethod 4: USB Direct Link Cable (Special Cable)" << endl;
        cout << "--------------------------------------------" << endl;
        cout << "1. Purchase a USB Direct Link cable with chip" << endl;
        cout << "2. Install driver that comes with the cable" << endl;
        cout << "3. Connect both PCs - driver creates network" << endl;
        
        cout << "\n========================================" << endl;
        cout << "\nAfter setup, run: file_transfer.exe -scan" << endl;
        cout << "to verify your connection is working!" << endl;
        cout << "========================================\n" << endl;
    }
};

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    if (argc > 1 && string(argv[1]) == "-setup") {
        USBNetworkHelper::printUSBSetupGuide();
    } else if (argc > 1 && string(argv[1]) == "-scan") {
        USBNetworkHelper::displayConnections();
        
        cout << "\n\nConnection Status:" << endl;
        if (USBNetworkHelper::isUSBNetworkAvailable()) {
            cout << "✓ USB network connection detected!" << endl;
        } else {
            cout << "✗ No USB network detected." << endl;
        }
        
        string recommendedIP = USBNetworkHelper::getRecommendedIP();
        if (recommendedIP != "Not Found") {
            cout << "\nRecommended IP for file transfer: " << recommendedIP << endl;
            cout << "Use this IP on the other PC when sending files." << endl;
        }
    } else {
        cout << "USB Network Helper" << endl;
        cout << "\nUsage:" << endl;
        cout << "  usb_helper.exe -setup   Show USB setup instructions" << endl;
        cout << "  usb_helper.exe -scan    Scan for available connections" << endl;
    }
    
    WSACleanup();
    return 0;
}
