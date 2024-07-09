#pragma once

#include "process_monitor_thread.h"
#include "gpu_monitor.h"
#include "config.h"
#include "logger.h"
#include "network_monitor.h"
#include "battery_monitor.h"
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <memory>
#include <sys/sysinfo.h>

class Display;

struct CPUCoreInfo {
    double utilization;
    double temperature;
    double clockSpeed;
};

struct GPUInfo {
    int index;
    std::string name;
    float temperature;
    float powerUsage;
    float fanSpeed;
    float gpuUtilization;
    float memoryUtilization;
    bool fanSpeedAvailable;
    float clockSpeed;
};

struct DiskPartitionInfo {
    std::string name;
    std::string mountPoint;
    unsigned long long totalSpace;
    unsigned long long usedSpace;
};

class SystemMonitor {
public:
    SystemMonitor(const Config& config, std::shared_ptr<Logger> logger, Display& display, bool nvml_available);
    bool initialize();
    void update();
    [[nodiscard]] double getCpuUsage() const;
    [[nodiscard]] const std::vector<CPUCoreInfo>& getCPUCoreInfo() const;
    [[nodiscard]] double getMemoryUsage() const;
    [[nodiscard]] double getDiskUsage() const;
    [[nodiscard]] const std::vector<DiskPartitionInfo>& getDiskPartitions() const;
    [[nodiscard]] std::vector<ProcessInfo> getProcesses() const;
    [[nodiscard]] std::vector<GPUInfo> getGPUInfo() const;
    [[nodiscard]] std::vector<NetworkInterface> getNetworkInterfaces() const;
    [[nodiscard]] bool isAlertTriggered() const;
    [[nodiscard]] bool isGPUMonitoringAvailable() const;
    [[nodiscard]] std::string getCpuModel() const;
    [[nodiscard]] unsigned long long getTotalMemory() const;
    [[nodiscard]] unsigned long long getTotalDiskSpace() const;
    [[nodiscard]] std::string getDiskName() const;
    [[nodiscard]] const BatteryMonitor& getBatteryMonitor() const;
    [[nodiscard]] long getUptime() const;
    void run();
    static const float GPU_TEMP_THRESHOLD;

private:
    double cpuUsage;
    std::vector<CPUCoreInfo> cpuCoreInfo;
    double memoryUsage;
    double diskUsage;
    std::vector<DiskPartitionInfo> diskPartitions;
    bool alertTriggered;
    bool nvml_available;
    bool gpuUnavailabilityLogged;
    const Config& config;
    ProcessMonitorThread processMonitorThread;
    GPUMonitor gpuMonitor;
    NetworkMonitor networkMonitor;
    BatteryMonitor batteryMonitor;
    std::shared_ptr<Logger> logger;
    Display& display;
    std::string cpuModel;
    unsigned long long totalMemory;
    unsigned long long totalDiskSpace;
    std::string diskName;
    long uptime;

    [[nodiscard]] double calculateCpuUsage();
    void updateCPUCoreInfo();
    [[nodiscard]] double calculateMemoryUsage();
    [[nodiscard]] double calculateDiskUsage();
    void updateDiskPartitions();
    [[nodiscard]] std::optional<std::vector<long long>> getSystemStats();
    void checkAlerts();
    bool initializeGPU();
    void initializeCpuInfo();
    void initializeMemoryInfo();
    void initializeDiskInfo();
    std::string getRootDeviceName();
    void updateUptime();
};
