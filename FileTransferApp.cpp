#ifndef UNICODE
#define UNICODE
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <iphlpapi.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <fstream>
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

#define PORT 8888
#define BUFFER_SIZE 4096

// Control IDs for Main Window
#define IDC_BTN_SENDER 2001
#define IDC_BTN_RECEIVER 2002

// Control IDs for Sender Window
#define IDC_SENDER_SERVER_IP 1003
#define IDC_SENDER_FILE_PATH 1004
#define IDC_SENDER_BROWSE_FILE 1005
#define IDC_SENDER_START_TRANSFER 1006
#define IDC_SENDER_STATUS_TEXT 1007
#define IDC_SENDER_PROGRESS_BAR 1008
#define IDC_SENDER_SCAN_NETWORK 1009
#define IDC_SENDER_NETWORK_LIST 1010
#define IDC_SENDER_STOP_TRANSFER 1011
#define IDC_SENDER_CONNECTION_STATUS 1012

// Control IDs for Receiver Window
#define IDC_RECEIVER_START 1103
#define IDC_RECEIVER_STOP 1104
#define IDC_RECEIVER_STATUS_TEXT 1107
#define IDC_RECEIVER_PROGRESS_BAR 1108
#define IDC_RECEIVER_MY_IP 1109
#define IDC_RECEIVER_CONNECTION_STATUS 1110

// Global variables - Main Window
HWND g_hMainWnd = NULL;
HWND g_hSenderWnd = NULL;
HWND g_hReceiverWnd = NULL;

// Sender Window Controls
HWND g_hSenderServerIP, g_hSenderFilePath;
HWND g_hSenderBrowseFile, g_hSenderStartTransfer, g_hSenderStopTransfer;
HWND g_hSenderStatusText, g_hSenderProgressBar;
HWND g_hSenderScanNetwork, g_hSenderNetworkList;
HWND g_hSenderConnectionStatus;
bool g_isSenderTransferring = false;
bool g_senderStopRequested = false;

// Receiver Window Controls
HWND g_hReceiverStart, g_hReceiverStop;
HWND g_hReceiverStatusText, g_hReceiverProgressBar;
HWND g_hReceiverMyIP, g_hReceiverConnectionStatus;
bool g_isReceiverListening = false;
bool g_receiverStopRequested = false;

// UI Resources
HBRUSH g_hBrushBackground = NULL;
HBRUSH g_hBrushPanel = NULL;
HFONT g_hFontTitle = NULL;
HFONT g_hFontNormal = NULL;
HFONT g_hFontBold = NULL;
HFONT g_hFontLarge = NULL;

// Animation variables
bool g_animatingSenderProgress = false;
bool g_animatingReceiverProgress = false;
int g_senderCurrentProgress = 0;
int g_senderTargetProgress = 0;
int g_receiverCurrentProgress = 0;
int g_receiverTargetProgress = 0;

// Network adapter structure
struct NetworkAdapter {
    std::wstring name;
    std::wstring ipAddress;
    std::wstring type;
    bool isConnected;
};

// Function declarations
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SenderWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ReceiverWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateSenderWindow(HINSTANCE hInstance);
void CreateReceiverWindow(HINSTANCE hInstance);
void BrowseForFileSender();
void StartSenderTransfer();
void StopSenderTransfer();
void StartReceiverListening();
void StopReceiverListening();
void ScanNetworkSender();
void UpdateSenderStatus(const std::wstring& message);
void UpdateReceiverStatus(const std::wstring& message);
void UpdateSenderProgress(int percentage);
void UpdateReceiverProgress(int percentage);
void UpdateSenderConnectionStatus(const std::wstring& status, COLORREF color);
void UpdateReceiverConnectionStatus(const std::wstring& status, COLORREF color);
void SendFile(const std::wstring& filePath, const std::wstring& serverIP);
void ReceiveFile();
std::vector<NetworkAdapter> GetNetworkAdapters();
std::wstring GetPrimaryIPAddress();

// WinMain entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Initialize Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    // Create fonts
    g_hFontTitle = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    
    g_hFontLarge = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    
    g_hFontNormal = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    
    g_hFontBold = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    
    g_hBrushBackground = CreateSolidBrush(RGB(245, 247, 250));
    g_hBrushPanel = CreateSolidBrush(RGB(255, 255, 255));
    
    // Register main window class
    const wchar_t MAIN_CLASS_NAME[] = L"FileTransferMainClass";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = MAIN_CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(240, 242, 245));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    RegisterClass(&wc);
    
    // Register sender window class
    const wchar_t SENDER_CLASS_NAME[] = L"FileTransferSenderClass";
    WNDCLASS wcSender = {};
    wcSender.lpfnWndProc = SenderWindowProc;
    wcSender.hInstance = hInstance;
    wcSender.lpszClassName = SENDER_CLASS_NAME;
    wcSender.hbrBackground = CreateSolidBrush(RGB(240, 242, 245));
    wcSender.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcSender.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    RegisterClass(&wcSender);
    
    // Register receiver window class
    const wchar_t RECEIVER_CLASS_NAME[] = L"FileTransferReceiverClass";
    WNDCLASS wcReceiver = {};
    wcReceiver.lpfnWndProc = ReceiverWindowProc;
    wcReceiver.hInstance = hInstance;
    wcReceiver.lpszClassName = RECEIVER_CLASS_NAME;
    wcReceiver.hbrBackground = CreateSolidBrush(RGB(240, 242, 245));
    wcReceiver.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcReceiver.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    RegisterClass(&wcReceiver);
    
    // Create main selection window
    g_hMainWnd = CreateWindowEx(
        0,
        MAIN_CLASS_NAME,
        L"PC to PC File Transfer",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 350,
        NULL, NULL, hInstance, NULL
    );
    
    if (g_hMainWnd == NULL) {
        return 0;
    }
    
    // Set application icons
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    SendMessage(g_hMainWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(g_hMainWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    
    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    
    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    if (g_hFontTitle) DeleteObject(g_hFontTitle);
    if (g_hFontLarge) DeleteObject(g_hFontLarge);
    if (g_hFontNormal) DeleteObject(g_hFontNormal);
    if (g_hFontBold) DeleteObject(g_hFontBold);
    if (g_hBrushBackground) DeleteObject(g_hBrushBackground);
    if (g_hBrushPanel) DeleteObject(g_hBrushPanel);
    
    WSACleanup();
    return 0;
}

// Main Window Procedure - Selection Window
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Title
            HWND hTitle = CreateWindow(L"STATIC", L"🚀 PC-to-PC File Transfer",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                20, 30, 440, 40, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)g_hFontTitle, TRUE);
            
            // Description
            HWND hDesc = CreateWindow(L"STATIC", L"Choose your transfer mode:",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                20, 80, 440, 25, hwnd, NULL, NULL, NULL);
            SendMessage(hDesc, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // Sender Button
            HWND hBtnSender = CreateWindow(L"BUTTON", L"📤 SEND FILES\n\nOpen sender window to transfer files to another PC",
                WS_VISIBLE | WS_CHILD | BS_MULTILINE | BS_CENTER,
                50, 130, 180, 120, hwnd, (HMENU)IDC_BTN_SENDER, NULL, NULL);
            SendMessage(hBtnSender, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // Receiver Button
            HWND hBtnReceiver = CreateWindow(L"BUTTON", L"📥 RECEIVE FILES\n\nOpen receiver window to accept files from another PC",
                WS_VISIBLE | WS_CHILD | BS_MULTILINE | BS_CENTER,
                250, 130, 180, 120, hwnd, (HMENU)IDC_BTN_RECEIVER, NULL, NULL);
            SendMessage(hBtnReceiver, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            return 0;
        }
        
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_BTN_SENDER:
                    if (g_hSenderWnd == NULL) {
                        CreateSenderWindow((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
                    } else {
                        ShowWindow(g_hSenderWnd, SW_RESTORE);
                        SetForegroundWindow(g_hSenderWnd);
                    }
                    break;
                    
                case IDC_BTN_RECEIVER:
                    if (g_hReceiverWnd == NULL) {
                        CreateReceiverWindow((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
                    } else {
                        ShowWindow(g_hReceiverWnd, SW_RESTORE);
                        SetForegroundWindow(g_hReceiverWnd);
                    }
                    break;
            }
            return 0;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(31, 41, 55));
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)g_hBrushBackground;
        }
        
        case WM_DESTROY:
            g_senderStopRequested = true;
            g_receiverStopRequested = true;
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Create Sender Window
void CreateSenderWindow(HINSTANCE hInstance) {
    g_hSenderWnd = CreateWindowEx(
        0,
        L"FileTransferSenderClass",
        L"📤 Send Files - C to C File Transfer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 750, 700,
        NULL, NULL, hInstance, NULL
    );
    
    ShowWindow(g_hSenderWnd, SW_SHOW);
    UpdateWindow(g_hSenderWnd);
}

// Create Receiver Window
void CreateReceiverWindow(HINSTANCE hInstance) {
    g_hReceiverWnd = CreateWindowEx(
        0,
        L"FileTransferReceiverClass",
        L"📥 Receive Files - C to C File Transfer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 550,
        NULL, NULL, hInstance, NULL
    );
    
    ShowWindow(g_hReceiverWnd, SW_SHOW);
    UpdateWindow(g_hReceiverWnd);
}

// Sender Window Procedure
LRESULT CALLBACK SenderWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            SetTimer(hwnd, 1, 50, NULL);
            
            // Title
            HWND hTitle = CreateWindow(L"STATIC", L"📤 Send Files to Another PC",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                20, 15, 690, 35, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)g_hFontLarge, TRUE);
            
            // Connection Status Box - Prominent
            HWND hConnGroup = CreateWindow(L"BUTTON", L"Connection Status",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 60, 690, 70, hwnd, NULL, NULL, NULL);
            SendMessage(hConnGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hSenderConnectionStatus = CreateWindow(L"STATIC", L"🔴 Not Connected",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                40, 90, 650, 30, hwnd, (HMENU)IDC_SENDER_CONNECTION_STATUS, NULL, NULL);
            SendMessage(g_hSenderConnectionStatus, WM_SETFONT, (WPARAM)g_hFontLarge, TRUE);
            
            // Network scan group box
            HWND hNetworkGroup = CreateWindow(L"BUTTON", L"Network Connections",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 145, 690, 150, hwnd, NULL, NULL, NULL);
            SendMessage(hNetworkGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            HWND hNetworkLabel = CreateWindow(L"STATIC", L"💻 Available Network Adapters (double-click to select):",
                WS_VISIBLE | WS_CHILD,
                35, 168, 500, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hNetworkLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hSenderScanNetwork = CreateWindow(L"BUTTON", L"🔍 Scan Network",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                545, 165, 150, 28, hwnd, (HMENU)IDC_SENDER_SCAN_NETWORK, NULL, NULL);
            SendMessage(g_hSenderScanNetwork, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hSenderNetworkList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOTIFY,
                35, 198, 660, 85, hwnd, (HMENU)IDC_SENDER_NETWORK_LIST, NULL, NULL);
            SendMessage(g_hSenderNetworkList, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // Target IP group box
            HWND hIPGroup = CreateWindow(L"BUTTON", L"Target PC Configuration",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 310, 690, 55, hwnd, NULL, NULL, NULL);
            SendMessage(hIPGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            HWND hIPLabel = CreateWindow(L"STATIC", L"🎯 Target PC IP Address:",
                WS_VISIBLE | WS_CHILD,
                35, 335, 200, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hIPLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hSenderServerIP = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
                240, 333, 450, 26, hwnd, (HMENU)IDC_SENDER_SERVER_IP, NULL, NULL);
            SendMessage(g_hSenderServerIP, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // File selection group box
            HWND hFileGroup = CreateWindow(L"BUTTON", L"File Selection",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 380, 690, 65, hwnd, NULL, NULL, NULL);
            SendMessage(hFileGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            HWND hFileLabel = CreateWindow(L"STATIC", L"📁 File to Send:",
                WS_VISIBLE | WS_CHILD,
                35, 403, 150, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hFileLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hSenderFilePath = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_READONLY,
                35, 415, 515, 22, hwnd, (HMENU)IDC_SENDER_FILE_PATH, NULL, NULL);
            SendMessage(g_hSenderFilePath, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hSenderBrowseFile = CreateWindow(L"BUTTON", L"📂 Browse...",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                560, 413, 135, 28, hwnd, (HMENU)IDC_SENDER_BROWSE_FILE, NULL, NULL);
            SendMessage(g_hSenderBrowseFile, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // Transfer buttons
            g_hSenderStartTransfer = CreateWindow(L"BUTTON", L"▶️ Start Transfer",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                20, 465, 330, 40, hwnd, (HMENU)IDC_SENDER_START_TRANSFER, NULL, NULL);
            SendMessage(g_hSenderStartTransfer, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hSenderStopTransfer = CreateWindow(L"BUTTON", L"⏹️ Stop Transfer",
                WS_CHILD | BS_PUSHBUTTON,
                380, 465, 330, 40, hwnd, (HMENU)IDC_SENDER_STOP_TRANSFER, NULL, NULL);
            SendMessage(g_hSenderStopTransfer, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            // Progress bar
            HWND hProgressLabel = CreateWindow(L"STATIC", L"⏳ Transfer Progress:",
                WS_VISIBLE | WS_CHILD,
                20, 520, 200, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hProgressLabel, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hSenderProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL,
                WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
                20, 545, 690, 28, hwnd, (HMENU)IDC_SENDER_PROGRESS_BAR, NULL, NULL);
            SendMessage(g_hSenderProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            
            // Status text
            HWND hStatusLabel = CreateWindow(L"STATIC", L"📋 Status Log:",
                WS_VISIBLE | WS_CHILD,
                20, 585, 200, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hStatusLabel, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hSenderStatusText = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"✅ Ready. Select a file and enter target IP, then click 'Start Transfer'.",
                WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                20, 608, 690, 85, hwnd, (HMENU)IDC_SENDER_STATUS_TEXT, NULL, NULL);
            SendMessage(g_hSenderStatusText, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            return 0;
        }
        
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_SENDER_BROWSE_FILE:
                    BrowseForFileSender();
                    break;
                    
                case IDC_SENDER_START_TRANSFER:
                    StartSenderTransfer();
                    break;
                    
                case IDC_SENDER_STOP_TRANSFER:
                    StopSenderTransfer();
                    break;
                    
                case IDC_SENDER_SCAN_NETWORK:
                    ScanNetworkSender();
                    break;
                    
                case IDC_SENDER_NETWORK_LIST:
                    if (HIWORD(wParam) == LBN_DBLCLK) {
                        int index = SendMessage(g_hSenderNetworkList, LB_GETCURSEL, 0, 0);
                        if (index != LB_ERR) {
                            wchar_t buffer[256];
                            SendMessage(g_hSenderNetworkList, LB_GETTEXT, index, (LPARAM)buffer);
                            std::wstring text(buffer);
                            
                            // Extract IP address from the string
                            size_t ipPos = text.find(L"IP: ");
                            if (ipPos != std::wstring::npos) {
                                std::wstring ip = text.substr(ipPos + 4);
                                size_t endPos = ip.find(L')');
                                if (endPos != std::wstring::npos) {
                                    ip = ip.substr(0, endPos);
                                }
                                SetWindowText(g_hSenderServerIP, ip.c_str());
                            }
                        }
                    }
                    break;
            }
            return 0;
        }
        
        case WM_TIMER: {
            if (wParam == 1) {
                // Smooth progress bar animation
                if (g_animatingSenderProgress && g_senderCurrentProgress != g_senderTargetProgress) {
                    if (g_senderCurrentProgress < g_senderTargetProgress) {
                        g_senderCurrentProgress = (std::min)(g_senderCurrentProgress + 2, g_senderTargetProgress);
                    } else {
                        g_senderCurrentProgress = (std::max)(g_senderCurrentProgress - 2, g_senderTargetProgress);
                    }
                    SendMessage(g_hSenderProgressBar, PBM_SETPOS, g_senderCurrentProgress, 0);
                }
            }
            return 0;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hControl = (HWND)lParam;
            
            // Special color for connection status
            if (hControl == g_hSenderConnectionStatus) {
                SetBkMode(hdcStatic, TRANSPARENT);
                return (LRESULT)g_hBrushBackground;
            }
            
            SetTextColor(hdcStatic, RGB(31, 41, 55));
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)g_hBrushBackground;
        }
        
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, RGB(17, 24, 39));
            SetBkColor(hdcEdit, RGB(255, 255, 255));
            return (LRESULT)g_hBrushPanel;
        }
        
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            g_senderStopRequested = true;
            g_hSenderWnd = NULL;
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Receiver Window Procedure
LRESULT CALLBACK ReceiverWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            SetTimer(hwnd, 2, 50, NULL);
            
            // Title
            HWND hTitle = CreateWindow(L"STATIC", L"📥 Receive Files from Another PC",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                20, 15, 640, 35, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)g_hFontLarge, TRUE);
            
            // Connection Status Box - Prominent
            HWND hConnGroup = CreateWindow(L"BUTTON", L"Connection Status",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 60, 640, 70, hwnd, NULL, NULL, NULL);
            SendMessage(hConnGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hReceiverConnectionStatus = CreateWindow(L"STATIC", L"🔴 Server Not Started",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                40, 90, 600, 30, hwnd, (HMENU)IDC_RECEIVER_CONNECTION_STATUS, NULL, NULL);
            SendMessage(g_hReceiverConnectionStatus, WM_SETFONT, (WPARAM)g_hFontLarge, TRUE);
            
            // My IP Address Box
            HWND hMyIPGroup = CreateWindow(L"BUTTON", L"Your PC's IP Address",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 145, 640, 80, hwnd, NULL, NULL, NULL);
            SendMessage(hMyIPGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            HWND hIPInstructions = CreateWindow(L"STATIC", L"Share this IP address with the sender PC:",
                WS_VISIBLE | WS_CHILD,
                35, 168, 600, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hIPInstructions, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hReceiverMyIP = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Click 'Start Listening' to see your IP",
                WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_READONLY | ES_CENTER,
                35, 193, 570, 26, hwnd, (HMENU)IDC_RECEIVER_MY_IP, NULL, NULL);
            SendMessage(g_hReceiverMyIP, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            // Control buttons
            g_hReceiverStart = CreateWindow(L"BUTTON", L"🚀 Start Listening",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                20, 245, 310, 40, hwnd, (HMENU)IDC_RECEIVER_START, NULL, NULL);
            SendMessage(g_hReceiverStart, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hReceiverStop = CreateWindow(L"BUTTON", L"⏹️ Stop Listening",
                WS_CHILD | BS_PUSHBUTTON,
                350, 245, 310, 40, hwnd, (HMENU)IDC_RECEIVER_STOP, NULL, NULL);
            SendMessage(g_hReceiverStop, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            // Progress bar
            HWND hProgressLabel = CreateWindow(L"STATIC", L"⏳ Receive Progress:",
                WS_VISIBLE | WS_CHILD,
                20, 305, 200, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hProgressLabel, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hReceiverProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL,
                WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
                20, 330, 640, 28, hwnd, (HMENU)IDC_RECEIVER_PROGRESS_BAR, NULL, NULL);
            SendMessage(g_hReceiverProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            
            // Status text
            HWND hStatusLabel = CreateWindow(L"STATIC", L"📋 Status Log:",
                WS_VISIBLE | WS_CHILD,
                20, 375, 200, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hStatusLabel, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hReceiverStatusText = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"✅ Ready. Click 'Start Listening' to wait for incoming files.",
                WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                20, 398, 640, 115, hwnd, (HMENU)IDC_RECEIVER_STATUS_TEXT, NULL, NULL);
            SendMessage(g_hReceiverStatusText, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            return 0;
        }
        
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_RECEIVER_START:
                    StartReceiverListening();
                    break;
                    
                case IDC_RECEIVER_STOP:
                    StopReceiverListening();
                    break;
            }
            return 0;
        }
        
        case WM_TIMER: {
            if (wParam == 2) {
                // Smooth progress bar animation
                if (g_animatingReceiverProgress && g_receiverCurrentProgress != g_receiverTargetProgress) {
                    if (g_receiverCurrentProgress < g_receiverTargetProgress) {
                        g_receiverCurrentProgress = (std::min)(g_receiverCurrentProgress + 2, g_receiverTargetProgress);
                    } else {
                        g_receiverCurrentProgress = (std::max)(g_receiverCurrentProgress - 2, g_receiverTargetProgress);
                    }
                    SendMessage(g_hReceiverProgressBar, PBM_SETPOS, g_receiverCurrentProgress, 0);
                }
            }
            return 0;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hControl = (HWND)lParam;
            
            // Special color for connection status
            if (hControl == g_hReceiverConnectionStatus) {
                SetBkMode(hdcStatic, TRANSPARENT);
                return (LRESULT)g_hBrushBackground;
            }
            
            SetTextColor(hdcStatic, RGB(31, 41, 55));
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)g_hBrushBackground;
        }
        
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, RGB(17, 24, 39));
            SetBkColor(hdcEdit, RGB(255, 255, 255));
            return (LRESULT)g_hBrushPanel;
        }
        
        case WM_DESTROY:
            KillTimer(hwnd, 2);
            g_receiverStopRequested = true;
            g_hReceiverWnd = NULL;
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Implementation Functions

void BrowseForFileSender() {
    OPENFILENAME ofn = {};
    wchar_t fileName[MAX_PATH] = L"";
    
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = g_hSenderWnd;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        SetWindowText(g_hSenderFilePath, fileName);
    }
}

void UpdateSenderStatus(const std::wstring& message) {
    wchar_t currentText[4096];
    GetWindowText(g_hSenderStatusText, currentText, 4096);
    
    std::wstring newText = currentText;
    newText += L"\r\n" + message;
    
    SetWindowText(g_hSenderStatusText, newText.c_str());
    
    // Scroll to bottom
    SendMessage(g_hSenderStatusText, EM_SETSEL, 0, -1);
    SendMessage(g_hSenderStatusText, EM_SETSEL, -1, -1);
    SendMessage(g_hSenderStatusText, EM_SCROLLCARET, 0, 0);
}

void UpdateReceiverStatus(const std::wstring& message) {
    wchar_t currentText[4096];
    GetWindowText(g_hReceiverStatusText, currentText, 4096);
    
    std::wstring newText = currentText;
    newText += L"\r\n" + message;
    
    SetWindowText(g_hReceiverStatusText, newText.c_str());
    
    // Scroll to bottom
    SendMessage(g_hReceiverStatusText, EM_SETSEL, 0, -1);
    SendMessage(g_hReceiverStatusText, EM_SETSEL, -1, -1);
    SendMessage(g_hReceiverStatusText, EM_SCROLLCARET, 0, 0);
}

void UpdateSenderProgress(int percentage) {
    g_senderTargetProgress = percentage;
    g_animatingSenderProgress = true;
    
    if (percentage == 100 || percentage == 0) {
        g_senderCurrentProgress = percentage;
        SendMessage(g_hSenderProgressBar, PBM_SETPOS, percentage, 0);
        g_animatingSenderProgress = false;
    }
}

void UpdateReceiverProgress(int percentage) {
    g_receiverTargetProgress = percentage;
    g_animatingReceiverProgress = true;
    
    if (percentage == 100 || percentage == 0) {
        g_receiverCurrentProgress = percentage;
        SendMessage(g_hReceiverProgressBar, PBM_SETPOS, percentage, 0);
        g_animatingReceiverProgress = false;
    }
}

void UpdateSenderConnectionStatus(const std::wstring& status, COLORREF color) {
    SetWindowText(g_hSenderConnectionStatus, status.c_str());
    // Change text color by redrawing
    InvalidateRect(g_hSenderConnectionStatus, NULL, TRUE);
}

void UpdateReceiverConnectionStatus(const std::wstring& status, COLORREF color) {
    SetWindowText(g_hReceiverConnectionStatus, status.c_str());
    // Change text color by redrawing
    InvalidateRect(g_hReceiverConnectionStatus, NULL, TRUE);
}

void StartSenderTransfer() {
    if (g_isSenderTransferring) {
        MessageBox(g_hSenderWnd, L"Transfer already in progress!", L"Error", MB_OK | MB_ICONWARNING);
        return;
    }
    
    wchar_t filePath[MAX_PATH];
    wchar_t serverIP[256];
    
    GetWindowText(g_hSenderFilePath, filePath, MAX_PATH);
    GetWindowText(g_hSenderServerIP, serverIP, 256);
    
    if (wcslen(filePath) == 0) {
        MessageBox(g_hSenderWnd, L"Please select a file to send!", L"Error", MB_OK | MB_ICONWARNING);
        return;
    }
    
    if (wcslen(serverIP) == 0) {
        MessageBox(g_hSenderWnd, L"Please enter the target PC's IP address!", L"Error", MB_OK | MB_ICONWARNING);
        return;
    }
    
    g_isSenderTransferring = true;
    g_senderStopRequested = false;
    EnableWindow(g_hSenderStartTransfer, FALSE);
    ShowWindow(g_hSenderStopTransfer, SW_SHOW);
    UpdateSenderProgress(0);
    SetWindowText(g_hSenderStatusText, L"");
    UpdateSenderConnectionStatus(L"🔵 Connecting...", RGB(59, 130, 246));
    UpdateSenderStatus(L"✨ Starting file transfer...");
    
    std::thread transferThread([filePath, serverIP]() {
        SendFile(filePath, serverIP);
        g_isSenderTransferring = false;
        EnableWindow(g_hSenderStartTransfer, TRUE);
        ShowWindow(g_hSenderStopTransfer, SW_HIDE);
    });
    transferThread.detach();
}

void StopSenderTransfer() {
    g_senderStopRequested = true;
    UpdateSenderStatus(L"⚠️ Stopping transfer...");
    UpdateSenderConnectionStatus(L"🔴 Disconnected", RGB(220, 38, 38));
}

void StartReceiverListening() {
    if (g_isReceiverListening) {
        MessageBox(g_hReceiverWnd, L"Server already listening!", L"Error", MB_OK | MB_ICONWARNING);
        return;
    }
    
    g_isReceiverListening = true;
    g_receiverStopRequested = false;
    EnableWindow(g_hReceiverStart, FALSE);
    ShowWindow(g_hReceiverStop, SW_SHOW);
    UpdateReceiverProgress(0);
    SetWindowText(g_hReceiverStatusText, L"");
    
    // Get and display IP address
    std::wstring myIP = GetPrimaryIPAddress();
    SetWindowText(g_hReceiverMyIP, myIP.c_str());
    
    UpdateReceiverConnectionStatus(L"🟡 Waiting for Connection...", RGB(234, 179, 8));
    UpdateReceiverStatus(L"🚀 Starting server...");
    UpdateReceiverStatus(L"⏳ Waiting for incoming connection...");
    UpdateReceiverStatus(L"📡 Your IP: " + myIP);
    
    std::thread transferThread([]() {
        ReceiveFile();
        g_isReceiverListening = false;
        EnableWindow(g_hReceiverStart, TRUE);
        ShowWindow(g_hReceiverStop, SW_HIDE);
    });
    transferThread.detach();
}

void StopReceiverListening() {
    g_receiverStopRequested = true;
    UpdateReceiverStatus(L"⚠️ Stopping server...");
    UpdateReceiverConnectionStatus(L"🔴 Server Stopped", RGB(220, 38, 38));
}

void SendFile(const std::wstring& filePath, const std::wstring& serverIP) {
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    
    // Convert wstring to string for socket operations
    std::string ipStr(serverIP.begin(), serverIP.end());
    std::string fileStr(filePath.begin(), filePath.end());
    
    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        UpdateSenderStatus(L"❌ Failed to create socket!");
        UpdateSenderConnectionStatus(L"🔴 Connection Failed", RGB(220, 38, 38));
        return;
    }
    
    // Prepare server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, ipStr.c_str(), &serverAddr.sin_addr) <= 0) {
        UpdateSenderStatus(L"❌ Invalid IP address!");
        UpdateSenderConnectionStatus(L"🔴 Invalid IP", RGB(220, 38, 38));
        closesocket(clientSocket);
        return;
    }
    
    UpdateSenderStatus(L"🔄 Connecting to " + serverIP + L"...");
    
    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        UpdateSenderStatus(L"❌ Connection failed! Make sure the other PC is listening.");
        UpdateSenderConnectionStatus(L"🔴 Connection Failed", RGB(220, 38, 38));
        closesocket(clientSocket);
        return;
    }
    
    UpdateSenderStatus(L"✅ Connected successfully!");
    UpdateSenderConnectionStatus(L"🟢 Connected to " + serverIP, RGB(34, 197, 94));
    
    // Open file
    std::ifstream file(filePath.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        UpdateSenderStatus(L"❌ Failed to open file!");
        UpdateSenderConnectionStatus(L"🔴 File Error", RGB(220, 38, 38));
        closesocket(clientSocket);
        return;
    }
    
    // Get file size
    long long fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Extract filename
    std::wstring fileName = filePath;
    size_t pos = fileName.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        fileName = fileName.substr(pos + 1);
    }
    
    std::string fileNameStr(fileName.begin(), fileName.end());
    
    // Send filename length
    int filenameLen = fileNameStr.length();
    send(clientSocket, (char*)&filenameLen, sizeof(filenameLen), 0);
    
    // Send filename
    send(clientSocket, fileNameStr.c_str(), filenameLen, 0);
    
    // Send file size
    send(clientSocket, (char*)&fileSize, sizeof(fileSize), 0);
    
    UpdateSenderStatus(L"📤 Sending file: " + fileName);
    UpdateSenderConnectionStatus(L"🟢 Transferring Data...", RGB(34, 197, 94));
    
    // Send file data
    char buffer[BUFFER_SIZE];
    long long totalSent = 0;
    
    while (!g_senderStopRequested && totalSent < fileSize) {
        file.read(buffer, BUFFER_SIZE);
        std::streamsize bytesRead = file.gcount();
        
        if (bytesRead <= 0) break;
        
        int bytesSent = send(clientSocket, buffer, bytesRead, 0);
        if (bytesSent <= 0) {
            UpdateSenderStatus(L"❌ Error sending data!");
            UpdateSenderConnectionStatus(L"🔴 Transfer Error", RGB(220, 38, 38));
            break;
        }
        
        totalSent += bytesSent;
        
        // Update progress
        int progress = (int)((totalSent * 100) / fileSize);
        UpdateSenderProgress(progress);
    }
    
    file.close();
    closesocket(clientSocket);
    
    if (g_senderStopRequested) {
        UpdateSenderStatus(L"⚠️ Transfer cancelled.");
        UpdateSenderConnectionStatus(L"🔴 Cancelled", RGB(220, 38, 38));
    } else {
        UpdateSenderStatus(L"🎉 File sent successfully!");
        UpdateSenderConnectionStatus(L"🟢 Transfer Complete!", RGB(34, 197, 94));
        UpdateSenderProgress(100);
        MessageBeep(MB_OK);
    }
}

void ReceiveFile() {
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    
    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        UpdateReceiverStatus(L"❌ Failed to create socket!");
        UpdateReceiverConnectionStatus(L"🔴 Socket Error", RGB(220, 38, 38));
        return;
    }
    
    // Prepare server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        UpdateReceiverStatus(L"❌ Bind failed! Port may be in use.");
        UpdateReceiverConnectionStatus(L"🔴 Bind Failed", RGB(220, 38, 38));
        closesocket(serverSocket);
        return;
    }
    
    // Listen for connections
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        UpdateReceiverStatus(L"❌ Listen failed!");
        UpdateReceiverConnectionStatus(L"🔴 Listen Failed", RGB(220, 38, 38));
        closesocket(serverSocket);
        return;
    }
    
    UpdateReceiverStatus(L"👂 Server listening on port 8888...");
    
    // Set socket to non-blocking for timeout
    u_long mode = 1;
    ioctlsocket(serverSocket, FIONBIO, &mode);
    
    // Accept with timeout check
    while (!g_receiverStopRequested) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket != INVALID_SOCKET) break;
        Sleep(100);
    }
    
    if (g_receiverStopRequested) {
        closesocket(serverSocket);
        UpdateReceiverStatus(L"⚠️ Server stopped.");
        UpdateReceiverConnectionStatus(L"🔴 Server Stopped", RGB(220, 38, 38));
        return;
    }
    
    // Set back to blocking
    mode = 0;
    ioctlsocket(clientSocket, FIONBIO, &mode);
    
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    
    std::wstring clientIPW(clientIP, clientIP + strlen(clientIP));
    UpdateReceiverStatus(L"🤝 Connection from: " + clientIPW);
    UpdateReceiverConnectionStatus(L"🟢 Connected from " + clientIPW, RGB(34, 197, 94));
    MessageBeep(MB_ICONINFORMATION);
    
    // Receive filename length
    int filenameLen;
    recv(clientSocket, (char*)&filenameLen, sizeof(filenameLen), 0);
    
    // Receive filename
    char* filename = new char[filenameLen + 1];
    recv(clientSocket, filename, filenameLen, 0);
    filename[filenameLen] = '\0';
    
    std::wstring fileNameW(filename, filename + strlen(filename));
    UpdateReceiverStatus(L"📥 Receiving file: " + fileNameW);
    UpdateReceiverConnectionStatus(L"🟢 Receiving: " + fileNameW, RGB(34, 197, 94));
    
    // Receive file size
    long long fileSize;
    recv(clientSocket, (char*)&fileSize, sizeof(fileSize), 0);
    
    // Save to Desktop by default
    wchar_t desktopPath[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath);
    std::wstring outputPath = std::wstring(desktopPath) + L"\\" + fileNameW;
    
    std::ofstream file(outputPath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        UpdateReceiverStatus(L"❌ Failed to create file!");
        UpdateReceiverConnectionStatus(L"🔴 File Error", RGB(220, 38, 38));
        delete[] filename;
        closesocket(clientSocket);
        closesocket(serverSocket);
        return;
    }
    
    // Receive file data
    char buffer[BUFFER_SIZE];
    long long totalReceived = 0;
    
    while (!g_receiverStopRequested && totalReceived < fileSize) {
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) break;
        
        file.write(buffer, bytesReceived);
        totalReceived += bytesReceived;
        
        // Update progress
        int progress = (int)((totalReceived * 100) / fileSize);
        UpdateReceiverProgress(progress);
    }
    
    file.close();
    delete[] filename;
    closesocket(clientSocket);
    closesocket(serverSocket);
    
    if (g_receiverStopRequested) {
        UpdateReceiverStatus(L"⚠️ Transfer cancelled.");
        UpdateReceiverConnectionStatus(L"🔴 Cancelled", RGB(220, 38, 38));
    } else {
        UpdateReceiverStatus(L"🎉 File received successfully!");
        UpdateReceiverStatus(L"💾 Saved to: " + outputPath);
        UpdateReceiverConnectionStatus(L"🟢 Transfer Complete!", RGB(34, 197, 94));
        UpdateReceiverProgress(100);
        MessageBeep(MB_OK);
    }
}

std::vector<NetworkAdapter> GetNetworkAdapters() {
    std::vector<NetworkAdapter> adapters;
    
    ULONG bufferSize = 0;
    GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &bufferSize);
    
    PIP_ADAPTER_ADDRESSES adapterAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
    
    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &bufferSize) == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES adapter = adapterAddresses;
        
        while (adapter) {
            NetworkAdapter netAdapter;
            netAdapter.name = adapter->FriendlyName;
            
            // Get IP address
            PIP_ADAPTER_UNICAST_ADDRESS unicast = adapter->FirstUnicastAddress;
            if (unicast) {
                struct sockaddr_in* addr = (struct sockaddr_in*)unicast->Address.lpSockaddr;
                char ipStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr->sin_addr, ipStr, INET_ADDRSTRLEN);
                netAdapter.ipAddress = std::wstring(ipStr, ipStr + strlen(ipStr));
            }
            
            // Determine adapter type
            if (adapter->IfType == IF_TYPE_ETHERNET_CSMACD) {
                netAdapter.type = L"Ethernet";
            } else if (adapter->IfType == IF_TYPE_USB) {
                netAdapter.type = L"USB";
            } else if (adapter->IfType == IF_TYPE_IEEE80211) {
                netAdapter.type = L"WiFi";
            } else {
                netAdapter.type = L"Other";
            }
            
            netAdapter.isConnected = (adapter->OperStatus == IfOperStatusUp);
            
            if (netAdapter.isConnected && !netAdapter.ipAddress.empty() && netAdapter.ipAddress != L"0.0.0.0") {
                adapters.push_back(netAdapter);
            }
            
            adapter = adapter->Next;
        }
    }
    
    free(adapterAddresses);
    return adapters;
}

std::wstring GetPrimaryIPAddress() {
    std::vector<NetworkAdapter> adapters = GetNetworkAdapters();
    
    // Prefer Ethernet, then WiFi, then USB, then any other
    for (const auto& adapter : adapters) {
        if (adapter.type == L"Ethernet") {
            return adapter.ipAddress;
        }
    }
    
    for (const auto& adapter : adapters) {
        if (adapter.type == L"WiFi") {
            return adapter.ipAddress;
        }
    }
    
    if (!adapters.empty()) {
        return adapters[0].ipAddress;
    }
    
    return L"No Active Network Connection";
}

void ScanNetworkSender() {
    SendMessage(g_hSenderNetworkList, LB_RESETCONTENT, 0, 0);
    UpdateSenderStatus(L"🔍 Scanning network adapters...");
    
    std::vector<NetworkAdapter> adapters = GetNetworkAdapters();
    
    for (const auto& adapter : adapters) {
        std::wstring icon = L"📶";
        if (adapter.type == L"Ethernet") icon = L"🔌";
        else if (adapter.type == L"WiFi") icon = L"📡";
        else if (adapter.type == L"USB") icon = L"🔗";
        
        std::wstring displayText = icon + L" " + adapter.name + L" (" + adapter.type + L" - IP: " + adapter.ipAddress + L")";
        SendMessage(g_hSenderNetworkList, LB_ADDSTRING, 0, (LPARAM)displayText.c_str());
    }
    
    int count = SendMessage(g_hSenderNetworkList, LB_GETCOUNT, 0, 0);
    UpdateSenderStatus(L"✅ Found " + std::to_wstring(count) + L" active network connections.");
    
    if (count > 0) {
        UpdateSenderStatus(L"💡 Tip: Double-click a connection to use its IP address.");
    }
}
