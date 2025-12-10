@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cl.exe /std:c++17 /EHsc /LD /O2 /DCPUUTILITYNATIVE_EXPORTS CPUUtilityNative.cpp /link /OUT:CPUUtilityNative.dll pdh.lib advapi32.lib
if %errorlevel% equ 0 (
    echo Build successful!
    copy CPUUtilityNative.dll "..\CSharpGUI\" 2>nul
) else (
    echo Build failed!
)

