@echo off
echo Quick Run Script - CPU Utility Hybrid
echo.

REM Check if DLL exists
if not exist "NativeDLL\CPUUtilityNative.dll" (
    echo Native DLL not found. Building...
    call build-all.bat
    if %errorlevel% neq 0 exit /b 1
)

REM Run in debug mode
cd CSharpGUI
dotnet run
cd ..
