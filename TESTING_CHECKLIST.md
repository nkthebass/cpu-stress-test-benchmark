# Testing Checklist - CPU Utility Hybrid

## Pre-Build Checks
- [ ] Visual Studio C++ Build Tools installed
- [ ] .NET 7 SDK installed
- [ ] `www/index.html` exists

## Build Phase
- [ ] Run `build-all.bat`
- [ ] C++ DLL compiles without errors (`CPUUtilityNative.dll` created)
- [ ] C# application builds without errors
- [ ] DLL is in output folder with EXE

## Launch Phase
- [ ] Application starts without crashing
- [ ] WebView2 window appears
- [ ] UI loads completely (no blank screen)
- [ ] No console errors

## Stress Test - Basic
- [ ] Click "Start Stress Test" (8 threads default)
- [ ] Task Manager shows ~95-100% CPU usage (vs ~70% in original)
- [ ] Process count shows "8 threads running"
- [ ] CPU temperature increases (if monitoring works)

## Stress Test - Pause/Resume
- [ ] Click "Pause Test" while running
- [ ] CPU usage drops to near 0%
- [ ] Status shows "Paused"
- [ ] Click "Resume Test"
- [ ] CPU usage returns to 95-100%

## Stress Test - Stop
- [ ] Click "Stop All"
- [ ] CPU usage drops to idle
- [ ] Thread count shows 0
- [ ] Can start again successfully

## Stress Test - Different Thread Counts
- [ ] Try 1 thread → ~12-15% CPU (1/8 of CPU)
- [ ] Try 2 threads → ~25% CPU
- [ ] Try 4 threads → ~50% CPU
- [ ] Try 16 threads → Should max out all cores

## Benchmark Test
- [ ] Click "Run Stable Benchmark"
- [ ] Progress bar moves smoothly
- [ ] Benchmark completes in 1-3 seconds (vs 10-30 in original)
- [ ] Score appears (should be 5-10x higher than original)
- [ ] Can run multiple times with consistent results

## Hardware Monitoring
- [ ] Right sidebar shows hardware graphs
- [ ] CPU Frequency graph updates every second
- [ ] Frequency changes when running stress test (should increase)
- [ ] CPU Load percentage shows correctly
- [ ] Temperature/Voltage may show 0 (normal without external tool)

## CPU Info
- [ ] CPU model name shows correctly
- [ ] Core count is correct
- [ ] Thread count is correct
- [ ] Max clock speed is reasonable

## Stability Tests
- [ ] Run stress test for 5 minutes → No crashes
- [ ] Run 10 benchmark cycles → No memory leaks
- [ ] Start/stop stress 10 times → No errors
- [ ] Close and restart app 5 times → Works consistently

## Performance Comparison (vs Original)

### Memory Usage (per thread)
- Original: ~15-30 MB per PowerShell process
- Hybrid: ~1-2 MB per native thread
- [ ] Hybrid uses 10-15x less memory ✓

### CPU Efficiency
- Original: 70-80% actual CPU load
- Hybrid: 95-100% actual CPU load
- [ ] Hybrid achieves 20-25% more efficiency ✓

### Responsiveness
- Original: 200-500ms to start stress
- Hybrid: 5-10ms to start stress
- [ ] Hybrid is 20-50x more responsive ✓

### Benchmark Speed
- Original: 10-30 seconds
- Hybrid: 1-3 seconds
- [ ] Hybrid is 5-10x faster ✓

## Known Issues / Limitations
- [ ] Temperature shows 0 (need MSR access or external tool)
- [ ] Voltage shows 0 (need MSR access)
- [ ] Package power shows 0 (need RAPL MSR access)
- [ ] Requires Visual Studio C++ to build

## Pass/Fail Criteria

### ✅ PASS if:
- Stress test achieves 95%+ CPU usage
- Benchmark runs 3x+ faster than original
- No crashes during 5-minute stress test
- Memory usage is significantly lower

### ❌ FAIL if:
- DLL won't build
- Application crashes on startup
- Stress test doesn't increase CPU usage
- Memory leaks during repeated tests

## Next Steps if PASS
1. Add MSR reading for temperature
2. Create installer with bundled DLL
3. Add more comprehensive benchmarks
4. Consider full Rust port

## Next Steps if FAIL
1. Check build errors in detail
2. Verify DLL is in correct location
3. Test with different thread counts
4. Check Task Manager for actual process behavior

---

**Current Status**: ⬜ Not Tested

Fill in checkboxes as you test each feature!
