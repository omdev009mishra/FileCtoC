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

// Control IDs
#define IDC_MODE_SERVER 1001
#define IDC_MODE_CLIENT 1002
#define IDC_SERVER_IP 1003
#define IDC_FILE_PATH 1004
#define IDC_BROWSE_FILE 1005
#define IDC_START_TRANSFER 1006
#define IDC_STATUS_TEXT 1007
#define IDC_PROGRESS_BAR 1008
#define IDC_SCAN_NETWORK 1009
#define IDC_NETWORK_LIST 1010
#define IDC_STOP_TRANSFER 1011

// Global variables
HWND g_hWnd;
HWND g_hModeServer, g_hModeClient;
HWND g_hServerIP, g_hFilePath;
HWND g_hBrowseFile, g_hStartTransfer, g_hStopTransfer;
HWND g_hStatusText, g_hProgressBar;
HWND g_hScanNetwork, g_hNetworkList;
bool g_isTransferring = false;
bool g_stopRequested = false;

// UI Colors
HBRUSH g_hBrushBackground = NULL;
HBRUSH g_hBrushPanel = NULL;
HBRUSH g_hBrushButton = NULL;
HFONT g_hFontTitle = NULL;
HFONT g_hFontNormal = NULL;
HFONT g_hFontBold = NULL;

// Animation variables
bool g_animatingProgress = false;
int g_currentProgress = 0;
int g_targetProgress = 0;
HWND g_hoveredButton = NULL;

// Network adapter structure
struct NetworkAdapter {
    std::wstring name;
    std::wstring ipAddress;
    std::wstring type;
    bool isConnected;
};

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void BrowseForFile();
void StartTransfer();
void StopTransfer();
void ScanNetwork();
void UpdateStatus(const std::wstring& message);
void UpdateProgress(int percentage);
void SendFile(const std::wstring& filePath, const std::wstring& serverIP);
void ReceiveFile();
std::vector<NetworkAdapter> GetNetworkAdapters();

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
    
    // Register window class
    const wchar_t CLASS_NAME[] = L"FileTransferAppClass";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(240, 242, 245));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    
    RegisterClass(&wc);
    
    // Create window with extended style for animations
    g_hWnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_COMPOSITED,
        CLASS_NAME,
        L"🚀 PC to PC File Transfer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 750, 680,
        NULL, NULL, hInstance, NULL
    );
    
    if (g_hWnd != NULL) {
        // Set application icons (both large and small)
        HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
        SendMessage(g_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(g_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        
        // Set initial opacity for fade-in animation
        SetLayeredWindowAttributes(g_hWnd, 0, 0, LWA_ALPHA);
    }
    
    if (g_hWnd == NULL) {
        return 0;
    }
    
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    // Fade-in animation
    for (int i = 0; i <= 255; i += 15) {
        SetLayeredWindowAttributes(g_hWnd, 0, i, LWA_ALPHA);
        Sleep(10);
    }
    SetLayeredWindowAttributes(g_hWnd, 0, 255, LWA_ALPHA);
    
    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    WSACleanup();
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Create custom fonts
            g_hFontTitle = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
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
            g_hBrushButton = CreateSolidBrush(RGB(0, 120, 215));
            
            // Set timer for smooth animations
            SetTimer(hwnd, 1, 20, NULL);
            
            // Title with larger font and icon - with shadow effect
            HWND hTitle = CreateWindow(L"STATIC", L"🚀 PC-to-PC File Transfer",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                20, 15, 690, 40, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)g_hFontTitle, TRUE);
            
            // Mode selection group box
            HWND hModeGroup = CreateWindow(L"BUTTON", L"Transfer Mode",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 65, 690, 55, hwnd, NULL, NULL, NULL);
            SendMessage(hModeGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            g_hModeServer = CreateWindow(L"BUTTON", L"📥 Receive Files (Server)",
                WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
                40, 90, 300, 22, hwnd, (HMENU)IDC_MODE_SERVER, NULL, NULL);
            SendMessage(g_hModeServer, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hModeClient = CreateWindow(L"BUTTON", L"📤 Send Files (Client)",
                WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                370, 90, 300, 22, hwnd, (HMENU)IDC_MODE_CLIENT, NULL, NULL);
            SendMessage(g_hModeClient, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // Default to server mode
            SendMessage(g_hModeServer, BM_SETCHECK, BST_CHECKED, 0);
            
            // Network scan group box
            HWND hNetworkGroup = CreateWindow(L"BUTTON", L"Network Connections",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 135, 690, 160, hwnd, NULL, NULL, NULL);
            SendMessage(hNetworkGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            HWND hNetworkLabel = CreateWindow(L"STATIC", L"💻 Available Network Adapters (double-click to select):",
                WS_VISIBLE | WS_CHILD,
                35, 158, 500, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hNetworkLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hScanNetwork = CreateWindow(L"BUTTON", L"🔍 Scan Network",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                545, 155, 150, 28, hwnd, (HMENU)IDC_SCAN_NETWORK, NULL, NULL);
            SendMessage(g_hScanNetwork, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hNetworkList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOTIFY,
                35, 188, 660, 95, hwnd, (HMENU)IDC_NETWORK_LIST, NULL, NULL);
            SendMessage(g_hNetworkList, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // Server IP group box
            HWND hIPGroup = CreateWindow(L"BUTTON", L"Target Configuration",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 310, 690, 55, hwnd, NULL, NULL, NULL);
            SendMessage(hIPGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            HWND hIPLabel = CreateWindow(L"STATIC", L"🎯 Target PC IP Address:",
                WS_VISIBLE | WS_CHILD,
                35, 335, 200, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hIPLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hServerIP = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
                240, 333, 290, 26, hwnd, (HMENU)IDC_SERVER_IP, NULL, NULL);
            SendMessage(g_hServerIP, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            HWND hIPHint = CreateWindow(L"STATIC", L"(Required for sending)",
                WS_VISIBLE | WS_CHILD | SS_RIGHT,
                540, 335, 155, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hIPHint, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // File selection group box
            HWND hFileGroup = CreateWindow(L"BUTTON", L"File Selection",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                20, 380, 690, 65, hwnd, NULL, NULL, NULL);
            SendMessage(hFileGroup, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            
            HWND hFileLabel = CreateWindow(L"STATIC", L"📁 File to Send:",
                WS_VISIBLE | WS_CHILD,
                35, 403, 150, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hFileLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            g_hFilePath = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_READONLY,
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
