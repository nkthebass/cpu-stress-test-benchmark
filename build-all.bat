@echo off
echo ========================================
echo   CPU Utility Hybrid - Build Script
echo ========================================
echo.

REM Step 1: Build Native DLL
echo [1/2] Building Native C++ DLL...
cd NativeDLL
call build.bat
if %errorlevel% neq 0 (
    echo ERROR: Native DLL build failed!
    cd ..
    pause
    exit /b 1
)
cd ..

echo.
echo [2/2] Building C# Application...
cd CSharpGUI
dotnet publish -c Release -f net7.0-windows -r win-x64 --self-contained false
if %errorlevel% neq 0 (
    echo ERROR: C# application build failed!
    cd ..
    pause
    exit /b 1
)
cd ..

echo.
echo ========================================
echo   BUILD SUCCESSFUL!
echo ========================================
echo.
echo Output location:
echo   CSharpGUI\bin\Release\net7.0-windows\win-x64\publish\
echo.
echo To run: CPUUtilityHybrid.exe
echo.
pause
