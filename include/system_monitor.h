// include/system_monitor.h
#pragma once

#include "process_monitor.h"
#include "config.h"
#include "logger.h"
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <memory>

class SystemMonitor {
public:
    SystemMonitor(const Config& config, std::shared_ptr<Logger> logger);
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
    std::shared_ptr<Logger> logger;

    [[nodiscard]] double calculateCpuUsage();
    [[nodiscard]] double calculateMemoryUsage();
    [[nodiscard]] double calculateDiskUsage();
    [[nodiscard]] std::optional<std::vector<long long>> getSystemStats();
    void checkAlerts();
};
