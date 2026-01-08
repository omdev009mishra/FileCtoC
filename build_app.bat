@echo off
echo =============================================
echo Building File Transfer GUI Application
echo =============================================
echo.

REM Check if icon exists, if not create it
if not exist app.ico (
    echo Icon file not found. Creating default icon...
    call create_icon.bat
    echo.
)

REM Check if g++ is available (MinGW)
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using MinGW g++...
    echo.
    
    echo Building GUI application...
    g++ FileTransferApp.cpp -o FileTransferApp.exe -municode -mwindows -lws2_32 -liphlpapi -lcomctl32 -lshlwapi -lshell32 -lole32 -static-libgcc -static-libstdc++
    
    if %ERRORLEVEL% EQU 0 (
        echo      SUCCESS: FileTransferApp.exe created
        echo.
        echo =============================================
        echo Build Complete!
        echo =============================================
        echo.
        echo Run: FileTransferApp.exe
        echo.
        echo The GUI application has been created!
        echo Double-click FileTransferApp.exe to launch.
        echo.
    ) else (
        echo      FAILED: FileTransferApp.exe
        echo.
        echo Build failed! Check the error messages above.
    )
    goto end
)

REM Check if cl is available (Visual Studio)
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using Visual Studio cl...
    echo.
    
    echo [1/2] Compiling resource file...
    rc resource.rc
    if %ERRORLEVEL% NEQ 0 (
        echo      WARNING: Resource compilation failed. Building without resources...
        echo [2/2] Building GUI application...
        cl FileTransferApp.cpp /Fe:FileTransferApp.exe /EHsc /D_UNICODE /DUNICODE /link user32.lib gdi32.lib comctl32.lib ws2_32.lib iphlpapi.lib shlwapi.lib shell32.lib comdlg32.lib /SUBSYSTEM:WINDOWS
    ) else (
        echo      SUCCESS: resource.res created
        echo [2/2] Building GUI application with resources...
        cl FileTransferApp.cpp resource.res /Fe:FileTransferApp.exe /EHsc /D_UNICODE /DUNICODE /link user32.lib gdi32.lib comctl32.lib ws2_32.lib iphlpapi.lib shlwapi.lib shell32.lib comdlg32.lib /SUBSYSTEM:WINDOWS
    )
    
    if %ERRORLEVEL% EQU 0 (
        echo      SUCCESS: FileTransferApp.exe created
        echo.
        echo =============================================
        echo Build Complete!
        echo =============================================
        echo.
        echo Run: FileTransferApp.exe
        echo.
        echo The GUI application has been created!
        echo Double-click FileTransferApp.exe to launch.
        echo.
        
        REM Clean up Visual Studio intermediate files
        del *.obj 2>nul
    ) else (
        echo      FAILED: FileTransferApp.exe
        echo.
        echo Build failed! Check the error messages above.
    )
    goto end
)

REM No compiler found
echo ERROR: No C++ compiler found!
echo.
echo Please install one of the following:
echo   1. MinGW-w64: https://www.mingw-w64.org/
echo   2. Visual Studio: https://visualstudio.microsoft.com/
echo.
echo After installation, make sure the compiler is in your PATH.
echo For MinGW: Add C:\mingw64\bin to your PATH
echo For Visual Studio: Run this from "Developer Command Prompt for VS"
echo.

:end
echo.
pause
