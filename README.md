# CPU Utility Hybrid

Hybrid architecture combining C# GUI with native C++ performance:
- **C# WinForms + WebView2**: UI layer (existing code)
- **C++ Native DLL**: High-performance stress testing and hardware monitoring

## Structure
- `CSharpGUI/` - C# application (modified from original)
- `NativeDLL/` - C++ DLL for stress & monitoring
- `www/` - Shared web assets

## Building
1. Build C++ DLL: Open `NativeDLL/NativeDLL.sln` in Visual Studio
2. Build C# app: `dotnet build CSharpGUI/`
3. DLL is automatically copied to output

## Benefits over Pure C#
- 10-20x faster stress test performance
- Direct CPU MSR access for temperature/voltage
- Multi-threaded stress with minimal overhead
- Still keeps familiar WebView2 UI
