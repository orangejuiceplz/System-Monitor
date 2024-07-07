#pragma once

#include "process_monitor_thread.h"
#include "gpu_monitor.h"
#include "config.h"
#include "logger.h"
#include "network_monitor.h"
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <memory>

class Display;

class SystemMonitor {
public:
    SystemMonitor(const Config& config, std::shared_ptr<Logger> logger, Display& display, bool nvml_available);
    bool initialize();
    void update();
    [[nodiscard]] double getCpuUsage() const;
    [[nodiscard]] double getMemoryUsage() const;
    [[nodiscard]] double getDiskUsage() const;
    [[nodiscard]] std::vector<ProcessInfo> getProcesses() const;
    [[nodiscard]] std::vector<GPUInfo> getGPUInfo() const;
    [[nodiscard]] std::vector<NetworkInterface> getNetworkInterfaces() const;
    [[nodiscard]] bool isAlertTriggered() const;
    [[nodiscard]] bool isGPUMonitoringAvailable() const;
    [[nodiscard]] std::string getCpuModel() const;
    [[nodiscard]] unsigned long long getTotalMemory() const;
    [[nodiscard]] unsigned long long getTotalDiskSpace() const;
    [[nodiscard]] std::string getDiskName() const;
    void run();
    static const float GPU_TEMP_THRESHOLD;

private:
    double cpuUsage;
    double memoryUsage;
    double diskUsage;
    bool alertTriggered;
    bool nvml_available;
    bool gpuUnavailabilityLogged;
    const Config& config;
    ProcessMonitorThread processMonitorThread;
    GPUMonitor gpuMonitor;
    NetworkMonitor networkMonitor;
    std::shared_ptr<Logger> logger;
    Display& display;
    std::string cpuModel;
    unsigned long long totalMemory;
    unsigned long long totalDiskSpace;
    std::string diskName;

    [[nodiscard]] double calculateCpuUsage();
    [[nodiscard]] double calculateMemoryUsage();
    [[nodiscard]] double calculateDiskUsage();
    [[nodiscard]] std::optional<std::vector<long long>> getSystemStats();
    void checkAlerts();
    bool initializeGPU();
    void initializeCpuInfo();
    void initializeMemoryInfo();
    void initializeDiskInfo();
};
