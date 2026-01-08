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
                35, 415, 515, 22, hwnd, (HMENU)IDC_FILE_PATH, NULL, NULL);
            SendMessage(g_hFilePath, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hBrowseFile = CreateWindow(L"BUTTON", L"📂 Browse...",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                560, 413, 135, 28, hwnd, (HMENU)IDC_BROWSE_FILE, NULL, NULL);
            SendMessage(g_hBrowseFile, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // Transfer buttons with owner draw for animations
            g_hStartTransfer = CreateWindow(L"BUTTON", L"▶️ Start Transfer",
                WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                20, 465, 330, 40, hwnd, (HMENU)IDC_START_TRANSFER, NULL, NULL);
            SendMessage(g_hStartTransfer, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hStopTransfer = CreateWindow(L"BUTTON", L"⏹️ Stop Transfer",
                WS_CHILD | BS_OWNERDRAW,
                380, 465, 330, 40, hwnd, (HMENU)IDC_STOP_TRANSFER, NULL, NULL);
            SendMessage(g_hStopTransfer, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            // Progress bar with label
            HWND hProgressLabel = CreateWindow(L"STATIC", L"⏳ Transfer Progress:",
                WS_VISIBLE | WS_CHILD,
                20, 520, 200, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hProgressLabel, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL,
                WS_VISIBLE | WS_CHILD | PBS_SMOOTH | PBS_MARQUEE,
                20, 545, 690, 28, hwnd, (HMENU)IDC_PROGRESS_BAR, NULL, NULL);
            SendMessage(g_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            SendMessage(g_hProgressBar, PBM_SETBARCOLOR, 0, RGB(16, 185, 129));
            SendMessage(g_hProgressBar, PBM_SETBKCOLOR, 0, RGB(229, 231, 235));
            
            // Status text with label
            HWND hStatusLabel = CreateWindow(L"STATIC", L"📋 Status Log:",
                WS_VISIBLE | WS_CHILD,
                20, 585, 200, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hStatusLabel, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hStatusText = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"✅ Ready. Select mode and click 'Start Transfer'.",
                WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                20, 608, 690, 85, hwnd, (HMENU)IDC_STATUS_TEXT, NULL, NULL);
            SendMessage(g_hStatusText, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            return 0;
        }
        
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_BROWSE_FILE:
                    BrowseForFile();
                    break;
                    
                case IDC_START_TRANSFER:
                    StartTransfer();
                    break;
                    
                case IDC_STOP_TRANSFER:
                    StopTransfer();
                    break;
                    
                case IDC_SCAN_NETWORK:
                    ScanNetwork();
                    break;
                    
                case IDC_NETWORK_LIST:
                    if (HIWORD(wParam) == LBN_DBLCLK) {
                        int index = SendMessage(g_hNetworkList, LB_GETCURSEL, 0, 0);
                        if (index != LB_ERR) {
                            wchar_t buffer[256];
                            SendMessage(g_hNetworkList, LB_GETTEXT, index, (LPARAM)buffer);
                            std::wstring text(buffer);
                            
                            // Extract IP address from the string
                            size_t ipPos = text.find(L"IP: ");
                            if (ipPos != std::wstring::npos) {
                                std::wstring ip = text.substr(ipPos + 4);
                                size_t endPos = ip.find(L')');
                                if (endPos != std::wstring::npos) {
                                    ip = ip.substr(0, endPos);
                                }
                                SetWindowText(g_hServerIP, ip.c_str());
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
                if (g_animatingProgress && g_currentProgress != g_targetProgress) {
                    if (g_currentProgress < g_targetProgress) {
                        g_currentProgress = (std::min)(g_currentProgress + 2, g_targetProgress);
                    } else {
                        g_currentProgress = (std::max)(g_currentProgress - 2, g_targetProgress);
                    }
                    SendMessage(g_hProgressBar, PBM_SETPOS, g_currentProgress, 0);
                }
            }
            return 0;
        }
        
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            if (dis->CtlType == ODT_BUTTON) {
                HDC hdc = dis->hDC;
                RECT rc = dis->rcItem;
                bool isPressed = (dis->itemState & ODS_SELECTED);
                bool isHovered = (g_hoveredButton == dis->hwndItem);
                bool isDisabled = (dis->itemState & ODS_DISABLED);
                
                // Determine colors based on button
                COLORREF bgColor, textColor;
                if (dis->CtlID == IDC_START_TRANSFER) {
                    if (isDisabled) {
                        bgColor = RGB(156, 163, 175);
                    } else if (isPressed) {
                        bgColor = RGB(5, 150, 105);
                    } else if (isHovered) {
                        bgColor = RGB(16, 185, 129);
                    } else {
                        bgColor = RGB(34, 197, 94);
                    }
                    textColor = RGB(255, 255, 255);
                } else if (dis->CtlID == IDC_STOP_TRANSFER) {
                    if (isDisabled) {
                        bgColor = RGB(156, 163, 175);
                    } else if (isPressed) {
                        bgColor = RGB(185, 28, 28);
                    } else if (isHovered) {
                        bgColor = RGB(239, 68, 68);
                    } else {
                        bgColor = RGB(220, 38, 38);
                    }
                    textColor = RGB(255, 255, 255);
                } else {
                    bgColor = RGB(0, 120, 215);
                    textColor = RGB(255, 255, 255);
                }
                
                // Draw rounded rectangle background
                HBRUSH hBrush = CreateSolidBrush(bgColor);
                HPEN hPen = CreatePen(PS_SOLID, 1, bgColor);
                SelectObject(hdc, hBrush);
                SelectObject(hdc, hPen);
                RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
                DeleteObject(hBrush);
                DeleteObject(hPen);
                
                // Draw text
                wchar_t text[256];
                GetWindowText(dis->hwndItem, text, 256);
                SetTextColor(hdc, textColor);
                SetBkMode(hdc, TRANSPARENT);
                SelectObject(hdc, g_hFontBold);
                DrawText(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                return TRUE;
            }
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);
            
            HWND hOldHover = g_hoveredButton;
            g_hoveredButton = NULL;
            
            RECT rc;
            if (GetWindowRect(g_hStartTransfer, &rc)) {
                POINT ptStart = {rc.left, rc.top};
                ScreenToClient(hwnd, &ptStart);
                rc.left = ptStart.x;
                rc.top = ptStart.y;
                rc.right = rc.left + (rc.right - rc.left);
                rc.bottom = rc.top + (rc.bottom - rc.top);
                if (PtInRect(&rc, pt)) {
                    g_hoveredButton = g_hStartTransfer;
                }
            }
            
            if (GetWindowRect(g_hStopTransfer, &rc)) {
                POINT ptStop = {rc.left, rc.top};
                ScreenToClient(hwnd, &ptStop);
                rc.left = ptStop.x;
                rc.top = ptStop.y;
                rc.right = rc.left + (rc.right - rc.left);
                rc.bottom = rc.top + (rc.bottom - rc.top);
                if (PtInRect(&rc, pt)) {
                    g_hoveredButton = g_hStopTransfer;
                }
            }
            
            if (hOldHover != g_hoveredButton) {
                if (hOldHover) InvalidateRect(hOldHover, NULL, FALSE);
                if (g_hoveredButton) InvalidateRect(g_hoveredButton, NULL, FALSE);
            }
            
            // Track mouse leave
            TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT)};
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            return 0;
        }
        
        case WM_MOUSELEAVE: {
            if (g_hoveredButton) {
                HWND hOld = g_hoveredButton;
                g_hoveredButton = NULL;
                InvalidateRect(hOld, NULL, FALSE);
            }
            return 0;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
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
            g_stopRequested = true;
            // Clean up resources
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hBrushBackground) DeleteObject(g_hBrushBackground);
            if (g_hBrushPanel) DeleteObject(g_hBrushPanel);
            if (g_hBrushButton) DeleteObject(g_hBrushButton);
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void BrowseForFile() {
    OPENFILENAME ofn = {};
    wchar_t fileName[MAX_PATH] = L"";
    
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = g_hWnd;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        SetWindowText(g_hFilePath, fileName);
    }
}

void UpdateStatus(const std::wstring& message) {
    wchar_t currentText[4096];
    GetWindowText(g_hStatusText, currentText, 4096);
    
    std::wstring newText = currentText;
    newText += L"\r\n" + message;
    
    SetWindowText(g_hStatusText, newText.c_str());
    
    // Scroll to bottom
    SendMessage(g_hStatusText, EM_SETSEL, 0, -1);
    SendMessage(g_hStatusText, EM_SETSEL, -1, -1);
    SendMessage(g_hStatusText, EM_SCROLLCARET, 0, 0);
}

void UpdateProgress(int percentage) {
    g_targetProgress = percentage;
    g_animatingProgress = true;
    
    // For instant updates on completion
    if (percentage == 100 || percentage == 0) {
        g_currentProgress = percentage;
        SendMessage(g_hProgressBar, PBM_SETPOS, percentage, 0);
        g_animatingProgress = false;
    }
}

void StartTransfer() {
    if (g_isTransferring) {
        MessageBox(g_hWnd, L"Transfer already in progress!", L"Error", MB_OK | MB_ICONWARNING);
        return;
    }
    
    bool isServer = (SendMessage(g_hModeServer, BM_GETCHECK, 0, 0) == BST_CHECKED);
    
    if (!isServer) {
        // Client mode - need file and IP
        wchar_t filePath[MAX_PATH];
        wchar_t serverIP[256];
        
        GetWindowText(g_hFilePath, filePath, MAX_PATH);
        GetWindowText(g_hServerIP, serverIP, 256);
        
        if (wcslen(filePath) == 0) {
            MessageBox(g_hWnd, L"Please select a file to send!", L"Error", MB_OK | MB_ICONWARNING);
            return;
        }
        
        if (wcslen(serverIP) == 0) {
            MessageBox(g_hWnd, L"Please enter the target PC's IP address!", L"Error", MB_OK | MB_ICONWARNING);
            return;
        }
        
        g_isTransferring = true;
        g_stopRequested = false;
        EnableWindow(g_hStartTransfer, FALSE);
        InvalidateRect(g_hStartTransfer, NULL, FALSE);
        ShowWindow(g_hStopTransfer, SW_SHOW);
        UpdateProgress(0);
        SetWindowText(g_hStatusText, L"");
        UpdateStatus(L"✨ Starting file transfer...");
        
        std::thread transferThread([filePath, serverIP]() {
            SendFile(filePath, serverIP);
            g_isTransferring = false;
            EnableWindow(g_hStartTransfer, TRUE);
            ShowWindow(g_hStopTransfer, SW_HIDE);
        });
        transferThread.detach();
        
    } else {
        // Server mode - just start listening
        g_isTransferring = true;
        g_stopRequested = false;
        EnableWindow(g_hStartTransfer, FALSE);
        InvalidateRect(g_hStartTransfer, NULL, FALSE);
        ShowWindow(g_hStopTransfer, SW_SHOW);
        UpdateProgress(0);
        SetWindowText(g_hStatusText, L"");
        UpdateStatus(L"🚀 Starting server...");
        UpdateStatus(L"⏳ Waiting for incoming connection...");
        
        std::thread transferThread([]() {
            ReceiveFile();
            g_isTransferring = false;
            EnableWindow(g_hStartTransfer, TRUE);
            ShowWindow(g_hStopTransfer, SW_HIDE);
        });
        transferThread.detach();
    }
}

void StopTransfer() {
    g_stopRequested = true;
    UpdateStatus(L"Stopping transfer...");
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
        UpdateStatus(L"Failed to create socket!");
        return;
    }
    
    // Prepare server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, ipStr.c_str(), &serverAddr.sin_addr) <= 0) {
        UpdateStatus(L"Invalid IP address!");
        closesocket(clientSocket);
        return;
    }
    
    UpdateStatus(L"🔄 Connecting to " + serverIP + L"...");
    
    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        UpdateStatus(L"❌ Connection failed! Make sure the other PC is listening.");
        closesocket(clientSocket);
        return;
    }
    
    UpdateStatus(L"✅ Connected successfully!");
    
    // Open file
    std::ifstream file(filePath.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        UpdateStatus(L"Failed to open file!");
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
    
    UpdateStatus(L"📤 Sending file: " + fileName);
    
    // Send file data
    char buffer[BUFFER_SIZE];
    long long totalSent = 0;
    
    while (!g_stopRequested && totalSent < fileSize) {
        file.read(buffer, BUFFER_SIZE);
        std::streamsize bytesRead = file.gcount();
        
        if (bytesRead <= 0) break;
        
        int bytesSent = send(clientSocket, buffer, bytesRead, 0);
        if (bytesSent <= 0) {
            UpdateStatus(L"Error sending data!");
            break;
        }
        
        totalSent += bytesSent;
        
        // Update progress
        int progress = (int)((totalSent * 100) / fileSize);
        UpdateProgress(progress);
    }
    
    file.close();
    closesocket(clientSocket);
    
    if (g_stopRequested) {
        UpdateStatus(L"⚠️ Transfer cancelled.");
    } else {
        UpdateStatus(L"🎉 File sent successfully!");
        UpdateProgress(100);
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
        UpdateStatus(L"Failed to create socket!");
        return;
    }
    
    // Prepare server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        UpdateStatus(L"Bind failed! Port may be in use.");
        closesocket(serverSocket);
        return;
    }
    
    // Listen for connections
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        UpdateStatus(L"Listen failed!");
        closesocket(serverSocket);
        return;
    }
    
    UpdateStatus(L"👂 Server listening on port 8888...");
    
    // Set socket to non-blocking for timeout
    u_long mode = 1;
    ioctlsocket(serverSocket, FIONBIO, &mode);
    
    // Accept with timeout check
    while (!g_stopRequested) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket != INVALID_SOCKET) break;
        Sleep(100);
    }
    
    if (g_stopRequested) {
        closesocket(serverSocket);
        UpdateStatus(L"Server stopped.");
        return;
    }
    
    // Set back to blocking
    mode = 0;
    ioctlsocket(clientSocket, FIONBIO, &mode);
    
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    
    std::wstring clientIPW(clientIP, clientIP + strlen(clientIP));
    UpdateStatus(L"🤝 Connection from: " + clientIPW);
    MessageBeep(MB_ICONINFORMATION);
    
    // Receive filename length
    int filenameLen;
    recv(clientSocket, (char*)&filenameLen, sizeof(filenameLen), 0);
    
    // Receive filename
    char* filename = new char[filenameLen + 1];
    recv(clientSocket, filename, filenameLen, 0);
    filename[filenameLen] = '\0';
    
    std::wstring fileNameW(filename, filename + strlen(filename));
    UpdateStatus(L"📥 Receiving file: " + fileNameW);
    
    // Receive file size
    long long fileSize;
    recv(clientSocket, (char*)&fileSize, sizeof(fileSize), 0);
    
    // Save to Desktop by default
    wchar_t desktopPath[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath);
    std::wstring outputPath = std::wstring(desktopPath) + L"\\" + fileNameW;
    
    std::ofstream file(outputPath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        UpdateStatus(L"Failed to create file!");
        delete[] filename;
        closesocket(clientSocket);
        closesocket(serverSocket);
        return;
    }
    
    // Receive file data
    char buffer[BUFFER_SIZE];
    long long totalReceived = 0;
    
    while (!g_stopRequested && totalReceived < fileSize) {
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) break;
        
        file.write(buffer, bytesReceived);
        totalReceived += bytesReceived;
        
        // Update progress
        int progress = (int)((totalReceived * 100) / fileSize);
        UpdateProgress(progress);
    }
    
    file.close();
    delete[] filename;
    closesocket(clientSocket);
    closesocket(serverSocket);
    
    if (g_stopRequested) {
        UpdateStatus(L"⚠️ Transfer cancelled.");
    } else {
        UpdateStatus(L"🎉 File received successfully!");
        UpdateStatus(L"💾 Saved to: " + outputPath);
        UpdateProgress(100);
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
            
            adapters.push_back(netAdapter);
            adapter = adapter->Next;
        }
    }
    
    free(adapterAddresses);
    return adapters;
}

void ScanNetwork() {
    SendMessage(g_hNetworkList, LB_RESETCONTENT, 0, 0);
    UpdateStatus(L"🔍 Scanning network adapters...");
    
    std::vector<NetworkAdapter> adapters = GetNetworkAdapters();
    
    for (const auto& adapter : adapters) {
        if (!adapter.isConnected || adapter.ipAddress.empty() || adapter.ipAddress == L"0.0.0.0") {
            continue;
        }
        
        std::wstring icon = L"📶";
        if (adapter.type == L"Ethernet") icon = L"🔌";
        else if (adapter.type == L"WiFi") icon = L"📡";
        else if (adapter.type == L"USB") icon = L"🔗";
        
        std::wstring displayText = icon + L" " + adapter.name + L" (" + adapter.type + L" - IP: " + adapter.ipAddress + L")";
        SendMessage(g_hNetworkList, LB_ADDSTRING, 0, (LPARAM)displayText.c_str());
    }
    
    int count = SendMessage(g_hNetworkList, LB_GETCOUNT, 0, 0);
    UpdateStatus(L"✅ Found " + std::to_wstring(count) + L" active network connections.");
    
    if (count > 0) {
        UpdateStatus(L"💡 Tip: Double-click a connection to use its IP address.");
    }
}
