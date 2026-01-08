@echo off
echo =============================================
echo Building File Transfer Program
echo Supports: Ethernet + USB Connections
echo =============================================
echo.

REM Check if g++ is available
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using MinGW g++...
    echo.
    echo [1/2] Building main transfer program...
    g++ file_transfer.cpp -o file_transfer.exe -lws2_32 -liphlpapi
    if %ERRORLEVEL% EQU 0 (
        echo      SUCCESS: file_transfer.exe created
    ) else (
        echo      FAILED: file_transfer.exe
        goto end
    )
    
    echo [2/2] Building USB network helper...
    g++ usb_network_setup.cpp -o usb_helper.exe -lws2_32 -liphlpapi
    if %ERRORLEVEL% EQU 0 (
        echo      SUCCESS: usb_helper.exe created
        echo.
        echo =============================================
        echo Build Complete!
        echo =============================================
        echo.
        echo Quick Start:
        echo   file_transfer.exe -ip        Show connections
        echo   usb_helper.exe -setup        USB setup guide
        echo.
    ) else (
        echo      FAILED: usb_helper.exe
    )
    goto end
)

REM Check if cl is available
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using Visual Studio cl...
    echo.
    echo [1/2] Building main transfer program...
    cl file_transfer.cpp /EHsc ws2_32.lib iphlpapi.lib
    if %ERRORLEVEL% EQU 0 (
        echo      SUCCESS: file_transfer.exe created
    ) else (
        echo      FAILED: file_transfer.exe
        goto end
    )
    
    echo [2/2] Building USB network helper...
    cl usb_network_setup.cpp /EHsc ws2_32.lib iphlpapi.lib
    if %ERRORLEVEL% EQU 0 (
        echo      SUCCESS: usb_helper.exe created
        echo.
        echo =============================================
        echo Build Complete!
        echo =============================================
        echo.
        echo Quick Start:
        echo   file_transfer.exe -ip        Show connections
        echo   usb_helper.exe -setup        USB setup guide
        echo.
    ) else (
        echo      FAILED: usb_helper.exe
    )
    goto end
)

echo ERROR: No C++ compiler found!
echo.
echo Please install one of the following:
echo   - MinGW-w64 (https://www.mingw-w64.org/)
echo   - Visual Studio (https://visualstudio.microsoft.com/)
echo.

:end
pause
