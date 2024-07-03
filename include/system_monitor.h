#pragma once

#include "process_monitor.h"
#include "config.h"
#include <string>
#include <vector>
#include <optional>
#include <filesystem>

class SystemMonitor {
public:
    SystemMonitor(const Config& config);
    void update();
    [[nodiscard]] double getCpuUsage() const;
    [[nodiscard]] double getMemoryUsage() const;
    [[nodiscard]] double getDiskUsage() const;
    [[nodiscard]] std::vector<ProcessInfo> getProcesses() const;
    [[nodiscard]] bool isAlertTriggered() const;

private:
    double cpuUsage;
    double memoryUsage;
    double diskUsage;
    bool alertTriggered;

    const Config& config;
    ProcessMonitor processMonitor;

    [[nodiscard]] double calculateCpuUsage();
    [[nodiscard]] double calculateMemoryUsage();
    [[nodiscard]] double calculateDiskUsage();
    [[nodiscard]] std::optional<std::vector<long long>> getSystemStats();
    void checkAlerts();
};
