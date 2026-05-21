@echo off
cd /d "%~dp0"

echo ====================================
echo   Building with GCC 14.2 (POSIX)...
echo ====================================
D:\MinGW-posix\mingw64\bin\g++.exe -std=c++17 -fexec-charset=UTF-8 -finput-charset=UTF-8 -I include src/*.cpp -o chess.exe 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ====================================
    echo   BUILD FAILED! See errors above.
    echo ====================================
    pause
    exit /b 1
)

echo ====================================
echo   Build OK, launching game...
echo ====================================
start "" chess.exe
echo Game started. Press any key to close this window...
pause
