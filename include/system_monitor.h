#pragma once

#include "process_monitor.h"
#include <string>
#include <vector>
#include <optional>
#include <filesystem>

class SystemMonitor {
public:
    SystemMonitor();
    void update();
    [[nodiscard]] double getCpuUsage() const;
    [[nodiscard]] double getMemoryUsage() const;
    [[nodiscard]] double getDiskUsage() const;
    [[nodiscard]] std::vector<ProcessInfo> getProcesses() const;

private:
    double cpuUsage;
    double memoryUsage;
    double diskUsage;

    [[nodiscard]] double calculateCpuUsage();
    [[nodiscard]] double calculateMemoryUsage();
    [[nodiscard]] double calculateDiskUsage();
    [[nodiscard]] std::optional<std::vector<long long>> getSystemStats();

    ProcessMonitor processMonitor;
};
