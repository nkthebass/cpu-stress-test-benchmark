#define CPUUTILITYNATIVE_EXPORTS
#include "CPUUtilityNative.h"
#include <windows.h>
#include <thread>
#include <vector>
#include <atomic>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <intrin.h>
#include <pdh.h>
#include <pdhmsg.h>

#pragma comment(lib, "pdh.lib")

// Global state for stress test
static std::vector<std::thread> stressThreads;
static std::atomic<bool> stressRunning(false);
static std::atomic<bool> stressPaused(false);

// Stress test worker function - pure CPU computation
void StressWorker() {
    // High-intensity computation designed to stress modern CPUs
    const int arraySize = 1024 * 1024; // 1MB of data per thread
    std::vector<double> dataA(arraySize);
    std::vector<double> dataB(arraySize);
    std::vector<double> result(arraySize);
    
    // Initialize with random-like data
    for (int i = 0; i < arraySize; i++) {
        dataA[i] = static_cast<double>(i) * 1.1;
        dataB[i] = static_cast<double>(i) * 0.9;
    }
    
    uint64_t iterations = 0;
    
    while (stressRunning) {
        if (!stressPaused) {
            // Mix of operations to stress different CPU units
            for (int i = 0; i < arraySize; i++) {
                // FPU stress - floating point operations
                double a = dataA[i];
                double b = dataB[i];
                
                // Complex math operations
                result[i] = std::sqrt(a * a + b * b);
                result[i] += std::sin(a) * std::cos(b);
                result[i] *= std::exp(std::log(a + 1.0));
                
                // Integer ALU stress
                uint64_t intVal = static_cast<uint64_t>(result[i] * 1000.0);
                intVal = (intVal * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
                result[i] = static_cast<double>(intVal);
            }
            
            // Memory operations to stress cache and memory controller
            for (int i = 0; i < arraySize / 2; i++) {
                std::swap(dataA[i], dataB[arraySize - i - 1]);
            }
            
            // Branch prediction stress
            for (int i = 0; i < 10000; i++) {
                double val = result[i % arraySize];
                if (val > 0.5) {
                    dataA[i % arraySize] += val * 0.1;
                } else {
                    dataB[i % arraySize] -= val * 0.1;
                }
            }
            
            iterations++;
            
            // Prevent compiler optimization
            if (iterations % 100 == 0) {
                volatile double preventOpt = result[iterations % arraySize];
                (void)preventOpt;
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

bool StartStressTest(int threadCount) {
    if (stressRunning) return false;
    
    stressRunning = true;
    stressPaused = false;
    stressThreads.clear();
    
    for (int i = 0; i < threadCount; i++) {
        stressThreads.emplace_back(StressWorker);
    }
    
    return true;
}

bool StopStressTest() {
    if (!stressRunning) return false;
    
    stressRunning = false;
    
    for (auto& thread : stressThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    stressThreads.clear();
    return true;
}

bool PauseStressTest() {
    if (!stressRunning) return false;
    stressPaused = true;
    return true;
}

bool ResumeStressTest() {
    if (!stressRunning) return false;
    stressPaused = false;
    return true;
}

int GetActiveThreadCount() {
    return static_cast<int>(stressThreads.size());
}

// Benchmark: Prime number calculation (single-threaded)
double RunSingleCoreBenchmark() {
    // Boost thread priority for consistency
    HANDLE hThread = GetCurrentThread();
    int originalPriority = GetThreadPriority(hThread);
    SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
    
    // Extended warm-up to stabilize CPU frequency and thermal state
    volatile int warmup = 0;
    for (int i = 0; i < 100000; i++) {
        warmup += static_cast<int>(std::sqrt(i * 2.5));
        if (i % 10000 == 0) {
            // Prevent aggressive optimization while warming up
            warmup += static_cast<int>(std::sin(i * 0.001));
        }
    }
    
    // Longer delay to let CPU frequency and thermals stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Run benchmark 5 times and take median to filter outliers
    double scores[5];
    for (int run = 0; run < 5; run++) {
        auto start = std::chrono::steady_clock::now();
        
        int primeCount = 0;
        const int limit = 100000;
        
        for (int num = 2; num <= limit; num++) {
            bool isPrime = true;
            int sqrtNum = static_cast<int>(std::sqrt(num));
            
            for (int i = 2; i <= sqrtNum; i++) {
                if (num % i == 0) {
                    isPrime = false;
                    break;
                }
            }
            
            if (isPrime) primeCount++;
        }
        
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        scores[run] = 1000.0 / elapsed.count();
        
        // Brief pause between runs
        if (run < 4) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    // Restore original priority
    SetThreadPriority(hThread, originalPriority);
    
    // Sort scores to find median
    std::sort(scores, scores + 5);
    
    // Return median score (middle value filters thermal throttling outliers)
    return scores[2];
}

// Multi-core benchmark: Prime number calculation using all threads
double RunMultiCoreBenchmark() {
    // Extended warm-up to stabilize CPU frequency and thermal state
    volatile int warmup = 0;
    for (int i = 0; i < 100000; i++) {
        warmup += static_cast<int>(std::sqrt(i * 2.5));
        if (i % 10000 == 0) {
            warmup += static_cast<int>(std::sin(i * 0.001));
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Get number of hardware threads
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // fallback
    
    // Run benchmark 5 times and take median
    double scores[5];
    for (int run = 0; run < 5; run++) {
        auto start = std::chrono::steady_clock::now();
        
        // Split work across threads - very large workload for ~20 second total
        const int limit = 5000000;  // 50x larger workload for proper timing
        const int rangePerThread = limit / numThreads;
        
        std::vector<std::thread> threads;
        std::vector<int> threadCounts(numThreads, 0);
        
        for (int t = 0; t < numThreads; t++) {
            int startNum = 2 + t * rangePerThread;
            int endNum = (t == numThreads - 1) ? limit : (startNum + rangePerThread);
            
            threads.emplace_back([startNum, endNum, &threadCounts, t]() {
                int primeCount = 0;
                for (int num = startNum; num <= endNum; num++) {
                    bool isPrime = true;
                    int sqrtNum = static_cast<int>(std::sqrt(num));
                    
                    for (int i = 2; i <= sqrtNum; i++) {
                        if (num % i == 0) {
                            isPrime = false;
                            break;
                        }
                    }
                    
                    if (isPrime) primeCount++;
                }
                threadCounts[t] = primeCount;
            });
        }
        
        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        scores[run] = 1000.0 / elapsed.count();
        
        if (run < 4) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    // Sort and return median
    std::sort(scores, scores + 5);
    return scores[2];
}

// Multi-core benchmark with progress callback
double RunMultiCoreBenchmarkWithProgress(ProgressCallback callback, int numRuns) {
    // Boost thread priority to minimize external interference
    HANDLE hThread = GetCurrentThread();
    DWORD originalPriority = GetThreadPriority(hThread);
    SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
    
    // Extended warm-up to stabilize CPU frequency and thermal state
    volatile int warmup = 0;
    for (int i = 0; i < 100000; i++) {
        warmup += static_cast<int>(std::sqrt(i * 2.5));
        if (i % 10000 == 0) {
            warmup += static_cast<int>(std::sin(i * 0.001));
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Get number of hardware threads
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // fallback
    
    // Clamp numRuns to valid range
    if (numRuns < 1) numRuns = 1;
    if (numRuns > 10) numRuns = 10;
    
    // Run benchmark numRuns times and take median
    std::vector<double> scores(numRuns);
    for (int run = 0; run < numRuns; run++) {
        if (callback) callback(run + 1, numRuns);
        
        auto start = std::chrono::steady_clock::now();
        
        // Split work across threads - very large workload for ~20 second total
        const int limit = 5000000;  // 50x larger workload for proper timing
        const int rangePerThread = limit / numThreads;
        
        std::vector<std::thread> threads;
        std::vector<int> threadCounts(numThreads, 0);
        
        for (int t = 0; t < numThreads; t++) {
            int startNum = 2 + t * rangePerThread;
            int endNum = (t == numThreads - 1) ? limit : (startNum + rangePerThread);
            
            threads.emplace_back([startNum, endNum, &threadCounts, t]() {
                int primeCount = 0;
                for (int num = startNum; num <= endNum; num++) {
                    bool isPrime = true;
                    int sqrtNum = static_cast<int>(std::sqrt(num));
                    
                    for (int i = 2; i <= sqrtNum; i++) {
                        if (num % i == 0) {
                            isPrime = false;
                            break;
                        }
                    }
                    
                    if (isPrime) primeCount++;
                }
                threadCounts[t] = primeCount;
            });
        }
        
        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        scores[run] = 100.0 / elapsed.count();  // Divide by 10 (1000/10=100)
        
        if (run < numRuns - 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    // Sort and return median
    std::sort(scores.begin(), scores.end());
    double medianScore = scores[numRuns / 2];
    
    // Restore original thread priority
    SetThreadPriority(hThread, originalPriority);
    
    return medianScore;
}

bool GetHardwareMetrics(HardwareMetrics* metrics) {
    if (!metrics) return false;
    
    // Initialize with defaults
    metrics->cpuLoad = -1.0;
    metrics->cpuFreqMHz = 0;
    metrics->tempC = 0.0;
    metrics->voltage = 0.0;
    metrics->packagePowerW = 0.0;
    metrics->isValid = false;
    
    // Get current CPU frequency from Performance Counter
    PDH_HQUERY freqQuery = nullptr;
    PDH_HCOUNTER perfCounter = nullptr;
    
    if (PdhOpenQuery(nullptr, 0, &freqQuery) == ERROR_SUCCESS) {
        // Use % Processor Performance which shows actual performance including turbo
        if (PdhAddEnglishCounterA(freqQuery, "\\Processor Information(_Total)\\% Processor Performance", 0, &perfCounter) == ERROR_SUCCESS) {
            PdhCollectQueryData(freqQuery);
            Sleep(100);
            PdhCollectQueryData(freqQuery);
            
            PDH_FMT_COUNTERVALUE counterVal;
            if (PdhGetFormattedCounterValue(perfCounter, PDH_FMT_DOUBLE, nullptr, &counterVal) == ERROR_SUCCESS) {
                // Get base frequency from registry
                HKEY hKey;
                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                    DWORD mhz = 0;
                    DWORD size = sizeof(DWORD);
                    if (RegQueryValueExA(hKey, "~MHz", nullptr, nullptr, (LPBYTE)&mhz, &size) == ERROR_SUCCESS && mhz > 0) {
                        // Calculate actual frequency: base * (performance% / 100)
                        metrics->cpuFreqMHz = static_cast<int>((counterVal.doubleValue / 100.0) * mhz);
                    }
                    RegCloseKey(hKey);
                }
            }
        }
        
        PdhCloseQuery(freqQuery);
    }
    
    // Get CPU load using PDH
    PDH_HQUERY loadQuery = nullptr;
    PDH_HCOUNTER loadCounter = nullptr;
    
    if (PdhOpenQuery(nullptr, 0, &loadQuery) == ERROR_SUCCESS) {
        if (PdhAddEnglishCounterA(loadQuery, "\\Processor(_Total)\\% Processor Time", 0, &loadCounter) == ERROR_SUCCESS) {
            PdhCollectQueryData(loadQuery);
            Sleep(100); // Need delay for accurate reading
            PdhCollectQueryData(loadQuery);
            
            PDH_FMT_COUNTERVALUE counterVal;
            if (PdhGetFormattedCounterValue(loadCounter, PDH_FMT_DOUBLE, nullptr, &counterVal) == ERROR_SUCCESS) {
                metrics->cpuLoad = counterVal.doubleValue;
            }
        }
        PdhCloseQuery(loadQuery);
    }
    
    // Temperature, voltage, and power require MSR access or WMI (complex)
    // For now, mark as valid if we got frequency or load
    metrics->isValid = (metrics->cpuFreqMHz > 0 || metrics->cpuLoad >= 0);
    
    return metrics->isValid;
}

bool GetCPUInfo(char* modelName, int modelNameSize, int* cores, int* threads, int* maxMHz) {
    if (!modelName || !cores || !threads || !maxMHz) return false;
    
    // Get CPU brand string using CPUID
    int cpuInfo[4] = {0};
    char brandString[64] = {0};
    
    __cpuid(cpuInfo, 0x80000000);
    unsigned int maxExtended = cpuInfo[0];
    
    if (maxExtended >= 0x80000004) {
        __cpuid((int*)(brandString), 0x80000002);
        __cpuid((int*)(brandString + 16), 0x80000003);
        __cpuid((int*)(brandString + 32), 0x80000004);
        
        // Trim leading spaces
        char* start = brandString;
        while (*start == ' ') start++;
        
        strncpy_s(modelName, modelNameSize, start, _TRUNCATE);
    }
    
    // Get core/thread count
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    *threads = sysInfo.dwNumberOfProcessors;
    
    // Estimate physical cores (this is approximate)
    *cores = *threads / 2;
    if (*cores == 0) *cores = *threads;
    
    // Try to extract frequency from CPU brand string (e.g., "Intel Core i5-1210H @ 2.50GHz")
    *maxMHz = 0;
    char* freqPos = strstr(modelName, "@");
    if (freqPos && strlen(freqPos) > 2) {
        float ghz = 0;
        if (sscanf_s(freqPos + 1, "%fGHz", &ghz) == 1 || sscanf_s(freqPos + 1, "%f GHz", &ghz) == 1) {
            *maxMHz = static_cast<int>(ghz * 1000);
        }
    }
    
    // Fallback to registry if brand string parsing failed
    if (*maxMHz == 0) {
        *maxMHz = 2400; // Default fallback
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD mhz = 0;
            DWORD size = sizeof(DWORD);
            if (RegQueryValueExA(hKey, "~MHz", nullptr, nullptr, (LPBYTE)&mhz, &size) == ERROR_SUCCESS && mhz > 0) {
                *maxMHz = mhz;
            }
            RegCloseKey(hKey);
        }
    }
    
    return true;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        StopStressTest();
        break;
    }
    return TRUE;
}
