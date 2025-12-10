# Quick Start Guide - CPU Utility Hybrid

## Prerequisites
1. **Visual Studio C++ Build Tools** (or full Visual Studio 2019/2022)
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Select "Desktop development with C++" workload
   
2. **.NET 7 SDK**
   - Should already be installed (you have it for original project)

## Building

### Option 1: Build Everything (Recommended for first time)
```batch
build-all.bat
```

This will:
1. Compile the C++ DLL (`CPUUtilityNative.dll`)
2. Build the C# application
3. Copy DLL to the output folder

### Option 2: Build Components Separately

**Build Native DLL only:**
```batch
cd NativeDLL
build.bat
cd ..
```

**Build C# GUI only:**
```batch
cd CSharpGUI
dotnet build
cd ..
```

## Running

### Quick Test Run:
```batch
run.bat
```

### Manual Run:
```batch
cd CSharpGUI\bin\Debug\net7.0-windows
CPUUtilityHybrid.exe
```

## Troubleshooting

### "cl.exe not found"
- Install Visual Studio C++ Build Tools
- Or open "Developer Command Prompt for VS" and run build from there

### "CPUUtilityNative.dll not found"
- Build the DLL first: `cd NativeDLL && build.bat`
- Check that DLL is in same folder as executable

### Web page doesn't load
- Check that `www` folder exists and contains `index.html`
- Check console output for path errors

## Testing the Performance

1. **Stress Test**: Click "Start Stress Test" and monitor Task Manager
   - Should see 95-100% CPU usage (vs 70-80% in original)
   - Lower memory usage per thread

2. **Benchmark**: Run benchmark and compare score
   - Should be 5-10x faster than original

3. **Hardware Monitoring**: Check the charts in right sidebar
   - Should show real-time CPU frequency
   - Should update every second

## Known Limitations (Current Version)

- Temperature and voltage reading require additional work (MSR access)
- No cross-platform support yet (Windows-only)
- Requires Visual Studio C++ tools to build

## Next Steps

If this works well, we can:
1. Add accurate temperature reading via MSR
2. Improve hardware sensor detection
3. Create installer with bundled DLL
4. Eventually port entirely to Rust for memory safety
