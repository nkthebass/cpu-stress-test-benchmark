@echo off
echo Building CPU Utility Native DLL...

REM Check for Visual Studio
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo Visual Studio C++ compiler not found in PATH
    echo Searching for Visual Studio installations...
    
    REM Try all common VS paths
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "%VSWHERE%" (
        for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            set "VSPATH=%%i"
        )
        if defined VSPATH (
            call "!VSPATH!\VC\Auxiliary\Build\vcvars64.bat"
            goto :compile
        )
    )
    
    REM Manual search in common locations
    for %%v in (2022 2019 2017) do (
        for %%e in (Community Professional Enterprise BuildTools) do (
            if exist "C:\Program Files\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat" (
                call "C:\Program Files\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat"
                goto :compile
            )
            if exist "C:\Program Files (x86)\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat" (
                call "C:\Program Files (x86)\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat"
                goto :compile
            )
        )
    )
    
    echo ERROR: Could not find Visual Studio installation
    echo Please install Visual Studio 2019/2022 with C++ Desktop Development workload
    echo Or run this from "Developer Command Prompt for VS"
    pause
    exit /b 1
)

:compile

REM Compile the DLL
cl.exe /std:c++17 /EHsc /LD /O2 ^
    /DCPUUTILITYNATIVE_EXPORTS ^
    CPUUtilityNative.cpp ^
    /link /OUT:CPUUtilityNative.dll ^
    pdh.lib

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Build successful!
    echo Output: CPUUtilityNative.dll
    echo ========================================
    
    REM Copy to CSharpGUI output if it exists
    if exist "..\CSharpGUI\bin\Debug\net7.0-windows\" (
        copy /Y CPUUtilityNative.dll "..\CSharpGUI\bin\Debug\net7.0-windows\"
    )
    if exist "..\CSharpGUI\bin\Release\net7.0-windows\win-x64\" (
        copy /Y CPUUtilityNative.dll "..\CSharpGUI\bin\Release\net7.0-windows\win-x64\"
    )
) else (
    echo.
    echo ========================================
    echo Build FAILED!
    echo ========================================
)

pause
