#pragma once

#ifdef CPUUTILITYNATIVE_EXPORTS
#define NATIVE_API __declspec(dllexport)
#else
#define NATIVE_API __declspec(dllimport)
#endif

#include <cstdint>

extern "C" {
    // Stress Test Functions
    NATIVE_API bool StartStressTest(int threadCount);
    NATIVE_API bool StopStressTest();
    NATIVE_API bool PauseStressTest();
    NATIVE_API bool ResumeStressTest();
    NATIVE_API int GetActiveThreadCount();

    // Benchmark Functions
    typedef void (*ProgressCallback)(int currentRun, int totalRuns);
    NATIVE_API double RunSingleCoreBenchmark();
    NATIVE_API double RunMultiCoreBenchmark();
    NATIVE_API double RunMultiCoreBenchmarkWithProgress(ProgressCallback callback, int numRuns);

    // Hardware Monitoring Functions
    struct HardwareMetrics {
        double cpuLoad;        // CPU utilization percentage
        int cpuFreqMHz;        // CPU frequency in MHz
        double tempC;          // CPU temperature in Celsius
        double voltage;        // CPU voltage in volts
        double packagePowerW;  // Package power in watts
        bool isValid;          // Whether data is valid
    };

    NATIVE_API bool GetHardwareMetrics(HardwareMetrics* metrics);
    NATIVE_API bool GetCPUInfo(char* modelName, int modelNameSize, int* cores, int* threads, int* maxMHz);
}
