using System;
using System.IO;
using System.Windows.Forms;
using Microsoft.Web.WebView2.WinForms;
using Microsoft.Web.WebView2.Core;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Text.Json;
using System.Text;
using LibreHardwareMonitor.Hardware;
using System.Linq;

namespace CPUUtilityHybrid
{
    // P/Invoke declarations for native DLL
    public static class NativeMethods
    {
        private const string DllName = "CPUUtilityNative.dll";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool StartStressTest(int threadCount);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool StopStressTest();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PauseStressTest();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool ResumeStressTest();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetActiveThreadCount();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern double RunSingleCoreBenchmark();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern double RunMultiCoreBenchmark();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ProgressCallback(int currentRun, int totalRuns);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern double RunMultiCoreBenchmarkWithProgress(ProgressCallback callback, int numRuns);

        [StructLayout(LayoutKind.Sequential)]
        public struct HardwareMetrics
        {
            public double cpuLoad;
            public int cpuFreqMHz;
            public double tempC;
            public double voltage;
            public double packagePowerW;
            [MarshalAs(UnmanagedType.I1)]
            public bool isValid;
        }

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetHardwareMetrics(ref HardwareMetrics metrics);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern bool GetCPUInfo(
            StringBuilder modelName, int modelNameSize,
            ref int cores, ref int threads, ref int maxMHz);
    }

    public class Form1 : Form
    {
        private WebView2 webView;
        private string appDir = string.Empty;
        private bool isPaused = false;
        private Computer computer;

        public Form1()
        {
            Text = "Xeno CPU utility 1.3.0";
            Width = 820;
            Height = 760;
            
            // Set the form icon
            try
            {
                string iconPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "app_icon.ico");
                if (File.Exists(iconPath))
                {
                    Icon = new Icon(iconPath);
                }
            }
            catch { /* Ignore icon loading errors */ }
            
            webView = new WebView2 { Dock = DockStyle.Fill };
            Controls.Add(webView);
            Load += Form1_Load;
            FormClosing += Form1_FormClosing;
            
            // Initialize hardware monitor
            computer = new Computer
            {
                IsCpuEnabled = true,
                IsMotherboardEnabled = true
            };
            computer.Open();
        }

        private void Form1_FormClosing(object? sender, FormClosingEventArgs e)
        {
            computer?.Close();
        }

        private async void Form1_Load(object? sender, EventArgs e)
        {
            appDir = AppDomain.CurrentDomain.BaseDirectory;
            var userDataFolder = Path.Combine(Path.GetTempPath(), "CPUUtilityHybrid", Guid.NewGuid().ToString());
            Directory.CreateDirectory(userDataFolder);

            var env = await CoreWebView2Environment.CreateAsync(null, userDataFolder);
            await webView.EnsureCoreWebView2Async(env);

            webView.CoreWebView2.WebMessageReceived += OnWebMessageReceived;
            webView.CoreWebView2.Settings.AreDevToolsEnabled = true;
            webView.CoreWebView2.Settings.AreDefaultContextMenusEnabled = true;

            var indexPath = Path.Combine(appDir, "www", "index.html");
            if (File.Exists(indexPath))
            {
                webView.CoreWebView2.Navigate(new Uri(indexPath).AbsoluteUri);
            }
            else
            {
                MessageBox.Show($"Could not find index.html at: {indexPath}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void OnWebMessageReceived(object? sender, CoreWebView2WebMessageReceivedEventArgs e)
        {
            try
            {
                var msg = JsonSerializer.Deserialize<JsonElement>(e.WebMessageAsJson);
                var id = msg.GetProperty("id").GetString() ?? "";
                var cmd = msg.GetProperty("cmd").GetString() ?? "";
                var args = msg.TryGetProperty("args", out var argsEl) ? argsEl : default;

                System.Diagnostics.Debug.WriteLine($"[Command] {cmd} (id: {id})");

                switch (cmd)
                {
                    case "startStress":
                        {
                            int threads = args.TryGetProperty("numProcesses", out var t) ? t.GetInt32() : 
                                         args.TryGetProperty("threads", out var t2) ? t2.GetInt32() : 4;
                            bool success = NativeMethods.StartStressTest(threads);
                            Reply(id, cmd, new { success, message = success ? "Stress test started" : "Failed to start" });
                        }
                        break;

                    case "stopStress":
                        {
                            bool success = NativeMethods.StopStressTest();
                            isPaused = false;
                            Reply(id, cmd, new { success, message = success ? "Stress test stopped" : "Failed to stop" });
                        }
                        break;

                    case "togglePauseResume":
                    case "pauseStress":
                        {
                            if (isPaused)
                            {
                                bool success = NativeMethods.ResumeStressTest();
                                if (success) isPaused = false;
                                Reply(id, cmd, new { success, message = success ? "Resumed" : "Failed to resume", isPaused = false });
                            }
                            else
                            {
                                bool success = NativeMethods.PauseStressTest();
                                if (success) isPaused = true;
                                Reply(id, cmd, new { success, message = success ? "Paused" : "Failed to pause", isPaused = true });
                            }
                        }
                        break;

                    case "resumeStress":
                        {
                            bool success = NativeMethods.ResumeStressTest();
                            if (success) isPaused = false;
                            Reply(id, cmd, new { success, message = success ? "Resumed" : "Failed to resume" });
                        }
                        break;

                    case "getStressStatus":
                        {
                            int count = NativeMethods.GetActiveThreadCount();
                            Reply(id, cmd, new { running = count > 0, paused = isPaused, threadCount = count });
                        }
                        break;

                    case "getCpuInfo":
                    case "getCPUInfo":
                        {
                            var modelName = new StringBuilder(256);
                            int cores = 0, threads = 0, maxMHz = 0;
                            bool success = NativeMethods.GetCPUInfo(modelName, 256, ref cores, ref threads, ref maxMHz);
                            
                            Reply(id, cmd, new
                            {
                                success,
                                name = modelName.ToString().Trim(),
                                model = modelName.ToString().Trim(),
                                cores,
                                threads,
                                logicalProcessors = threads,
                                maxClockMHz = maxMHz,
                                maxClockGHz = maxMHz / 1000.0
                            });
                        }
                        break;

                    case "getAppDir":
                        {
                            Reply(id, cmd, appDir);
                        }
                        break;

                    case "fileExists":
                        {
                            string path = args.GetProperty("path").GetString() ?? "";
                            bool exists = File.Exists(path);
                            Reply(id, cmd, exists);
                        }
                        break;

                    case "getHardwareSensors":
                        {
                            Task.Run(() =>
                            {
                                var sensors = new System.Collections.Generic.List<object>();
                                try
                                {
                                    foreach (var hardware in computer.Hardware)
                                    {
                                        hardware.Update();
                                        foreach (var sensor in hardware.Sensors)
                                        {
                                            sensors.Add(new
                                            {
                                                Hardware = hardware.Name,
                                                HardwareType = hardware.HardwareType.ToString(),
                                                Name = sensor.Name,
                                                Type = sensor.SensorType.ToString(),
                                                Value = sensor.Value
                                            });
                                        }
                                    }
                                }
                                catch { }
                                BeginInvoke(new Action(() => Reply(id, cmd, sensors)));
                            });
                        }
                        break;

                    case "runMultiCoreBenchmark":
                        {
                            Task.Run(() =>
                            {
                                // Get numRuns from the request, default to 3
                                int numRuns = 3;
                                try
                                {
                                    if (args.ValueKind != JsonValueKind.Undefined && args.ValueKind != JsonValueKind.Null)
                                    {
                                        if (args.TryGetProperty("numRuns", out JsonElement numRunsElement))
                                        {
                                            numRuns = numRunsElement.GetInt32();
                                        }
                                    }
                                }
                                catch { /* Use default */ }

                                var runScores = new System.Collections.Generic.List<double>();
                                NativeMethods.ProgressCallback callback = (current, total) =>
                                {
                                    BeginInvoke(new Action(() =>
                                    {
                                        Broadcast("benchmarkProgress", new { currentRun = current, totalRuns = total });
                                    }));
                                };
                                double score = NativeMethods.RunMultiCoreBenchmarkWithProgress(callback, numRuns);
                                // Note: We can't get individual scores from current C++ implementation
                                // For now, return the median score only
                                BeginInvoke(new Action(() => Reply(id, cmd, new { score, success = score > 0 })));
                            });
                        }
                        break;

                    case "getHardwareMetrics":
                        {
                            Task.Run(() =>
                            {
                                // Get basic metrics from native DLL
                                var metrics = new NativeMethods.HardwareMetrics();
                                NativeMethods.GetHardwareMetrics(ref metrics);
                                
                                // Get additional metrics from LibreHardwareMonitor
                                double? tempC = null;
                                double? voltage = null;
                                double? power = null;
                                
                                try
                                {
                                    foreach (var hardware in computer.Hardware)
                                    {
                                        hardware.Update();
                                        
                                        if (hardware.HardwareType == HardwareType.Cpu)
                                        {
                                            foreach (var sensor in hardware.Sensors)
                                            {
                                                // Temperature: prefer Package, fallback to any CPU temp
                                                if (sensor.SensorType == SensorType.Temperature)
                                                {
                                                    if (sensor.Name.Contains("Package") || sensor.Name.Contains("CPU"))
                                                    {
                                                        if (!tempC.HasValue || sensor.Name.Contains("Package"))
                                                            tempC = sensor.Value;
                                                    }
                                                }
                                                // Voltage: take first available CPU voltage
                                                else if (sensor.SensorType == SensorType.Voltage && !voltage.HasValue)
                                                {
                                                    voltage = sensor.Value;
                                                    System.Diagnostics.Debug.WriteLine($"Found voltage sensor: {sensor.Name} = {sensor.Value}V");
                                                }
                                                // Power: prefer Package power
                                                else if (sensor.SensorType == SensorType.Power)
                                                {
                                                    if (sensor.Name.Contains("Package") || sensor.Name.Contains("CPU"))
                                                    {
                                                        if (!power.HasValue || sensor.Name.Contains("Package"))
                                                            power = sensor.Value;
                                                    }
                                                }
                                            }
                                        }
                                        // Also check motherboard for CPU voltage
                                        else if (hardware.HardwareType == HardwareType.Motherboard && !voltage.HasValue)
                                        {
                                            foreach (var sensor in hardware.Sensors)
                                            {
                                                if (sensor.SensorType == SensorType.Voltage && 
                                                    (sensor.Name.Contains("CPU") || sensor.Name.Contains("VCore") || sensor.Name.Contains("Vcore")))
                                                {
                                                    voltage = sensor.Value;
                                                    System.Diagnostics.Debug.WriteLine($"Found motherboard voltage sensor: {sensor.Name} = {sensor.Value}V");
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                catch (Exception ex) 
                                { 
                                    System.Diagnostics.Debug.WriteLine($"Error reading sensors: {ex.Message}");
                                }
                                
                                // Get RAM usage
                                var ramUsedGB = 0.0;
                                var ramTotalGB = 0.0;
                                var ramUsagePercent = 0.0;
                                try
                                {
                                    var ci = new Microsoft.VisualBasic.Devices.ComputerInfo();
                                    ramTotalGB = ci.TotalPhysicalMemory / (1024.0 * 1024.0 * 1024.0);
                                    var availableGB = ci.AvailablePhysicalMemory / (1024.0 * 1024.0 * 1024.0);
                                    ramUsedGB = ramTotalGB - availableGB;
                                    ramUsagePercent = (ramUsedGB / ramTotalGB) * 100.0;
                                }
                                catch { }
                                
                                var result = new
                                {
                                    CpuLoad = metrics.cpuLoad,
                                    CpuFreqMHz = metrics.cpuFreqMHz,
                                    TempC = tempC,
                                    Voltage = voltage,
                                    PackagePowerW = power,
                                    RamUsedGB = ramUsedGB,
                                    RamTotalGB = ramTotalGB,
                                    RamUsagePercent = ramUsagePercent,
                                    Timestamp = DateTime.UtcNow
                                };
                                
                                BeginInvoke(new Action(() => Reply(id, cmd, result)));
                            });
                        }
                        break;

                    default:
                        Reply(id, cmd, new { error = "Unknown command" });
                        break;
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error handling message: {ex.Message}");
            }
        }

        private void Reply(string id, string command, object result)
        {
            try
            {
                var response = new { id, replyTo = command, result };
                var json = JsonSerializer.Serialize(response);
                System.Diagnostics.Debug.WriteLine($"[Reply] {command}: {json}");
                webView?.CoreWebView2?.PostWebMessageAsJson(json);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"[Reply Error] {ex.Message}");
            }
        }

        private void Broadcast(string eventName, object data)
        {
            try
            {
                var message = new { broadcast = eventName, result = data };
                var json = JsonSerializer.Serialize(message);
                System.Diagnostics.Debug.WriteLine($"[Broadcast] {eventName}: {json}");
                webView?.CoreWebView2?.PostWebMessageAsJson(json);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"[Broadcast Error] {ex.Message}");
            }
        }
    }
}
