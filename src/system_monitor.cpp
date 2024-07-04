#include "../include/system_monitor.h"
#include "../include/display.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <filesystem>

const float SystemMonitor::GPU_TEMP_THRESHOLD = 80.0f;

SystemMonitor::SystemMonitor(const Config& config, std::shared_ptr<Logger> logger, Display& display, bool nvml_available)
    : cpuUsage(0), memoryUsage(0), diskUsage(0), alertTriggered(false), 
      nvml_available(nvml_available), config(config), logger(logger), display(display) {}

bool SystemMonitor::initialize() {
    if (nvml_available) {
        try {
            if (!gpuMonitor.initialize()) {
                logger->log(LogLevel::ERROR, "Failed to initialize GPU monitor");
                nvml_available = false;
            }
        } catch (const std::exception& e) {
            logger->log(LogLevel::ERROR, "Exception during GPU monitor initialization: " + std::string(e.what()));
            nvml_available = false;
        } catch (...) {
            logger->log(LogLevel::ERROR, "Unknown exception during GPU monitor initialization");
            nvml_available = false;
        }
    } else {
        logger->log(LogLevel::INFO, "GPU monitoring is disabled");
    }
    return true;
}

void SystemMonitor::update() {
    cpuUsage = calculateCpuUsage();
    memoryUsage = calculateMemoryUsage();
    diskUsage = calculateDiskUsage();
    processMonitor.update();
    if (nvml_available) {
        gpuMonitor.update();
    }
    checkAlerts();

    std::string logMessage = "System update: CPU=" + std::to_string(cpuUsage) + 
                             "%, Memory=" + std::to_string(memoryUsage) + 
                             "%, Disk=" + std::to_string(diskUsage) + "%";
    logger->log(LogLevel::INFO, logMessage);
    display.addLogMessage(logMessage);

    // Get and sort processes
    auto processes = getProcesses();

    // Log top 5 processes
    for (size_t i = 0; i < std::min<size_t>(5, processes.size()); ++i) {
        const auto& p = processes[i];
        std::string processLog = "Top Process " + std::to_string(i+1) + ": " + p.name + 
                                 " (PID: " + std::to_string(p.pid) + ") CPU: " + 
                                 std::to_string(p.cpuUsage) + "%, Mem: " + 
                                 std::to_string(p.memoryUsage) + " MB, Overall: " +
                                 std::to_string(p.overallUsage) + "%";
        logger->log(LogLevel::INFO, processLog);
        display.addLogMessage(processLog);
    }

    if (nvml_available) {
        auto gpuInfos = gpuMonitor.getGPUInfo();
        for (const auto& gpu : gpuInfos) {
            std::string gpuLog = "GPU " + std::to_string(gpu.index) + ": " + gpu.name +
                                 " Temp: " + std::to_string(gpu.temperature) + "C" +
                                 " Util: " + std::to_string(gpu.gpuUtilization) + "%" +
                                 " Mem: " + std::to_string(gpu.memoryUtilization) + "%";
            logger->log(LogLevel::INFO, gpuLog);
            display.addLogMessage(gpuLog);
        }
    }
}

double SystemMonitor::getCpuUsage() const {
    return cpuUsage;
}

double SystemMonitor::getMemoryUsage() const {
    return memoryUsage;
}

double SystemMonitor::getDiskUsage() const {
    return diskUsage;
}

std::vector<ProcessInfo> SystemMonitor::getProcesses() const {
    return processMonitor.getProcesses();
}

std::vector<GPUInfo> SystemMonitor::getGPUInfo() const {
    if (nvml_available) {
        return gpuMonitor.getGPUInfo();
    }
    return {};
}

bool SystemMonitor::isAlertTriggered() const {
    return alertTriggered;
}

bool SystemMonitor::isGPUMonitoringAvailable() const {
    return nvml_available;
}

double SystemMonitor::calculateCpuUsage() {
    static auto lastStats = getSystemStats();
    auto currentStats = getSystemStats();

    if (!lastStats || !currentStats || lastStats->size() < 4 || currentStats->size() < 4) {
        return 0.0;
    }

    auto lastIdle = (*lastStats)[3];
    auto lastTotal = std::accumulate(lastStats->begin(), lastStats->end(), 0LL);

    auto currentIdle = (*currentStats)[3];
    auto currentTotal = std::accumulate(currentStats->begin(), currentStats->end(), 0LL);

    auto idleDelta = currentIdle - lastIdle;
    auto totalDelta = currentTotal - lastTotal;

    lastStats = currentStats;

    if (totalDelta == 0) {
        return 0.0;
    }

    return 100.0 * (1.0 - static_cast<double>(idleDelta) / totalDelta);
}

double SystemMonitor::calculateMemoryUsage() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    long long totalMem = 0, freeMem = 0, buffers = 0, cached = 0;

    while (std::getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string key;
        long long value;

        if (iss >> key >> value) {
            if (key == "MemTotal:") totalMem = value;
            else if (key == "MemFree:") freeMem = value;
            else if (key == "Buffers:") buffers = value;
            else if (key == "Cached:") cached = value;
        }
    }

    auto usedMem = totalMem - freeMem - buffers - cached;
    return 100.0 * static_cast<double>(usedMem) / totalMem;
}

double SystemMonitor::calculateDiskUsage() {
    try {
        auto space = std::filesystem::space("/");
        auto totalSpace = space.capacity;
        auto freeSpace = space.free;
        auto usedSpace = totalSpace - freeSpace;

        return 100.0 * static_cast<double>(usedSpace) / totalSpace;
    } catch (const std::filesystem::filesystem_error& e) {
        logger->log(LogLevel::ERROR, "Error calculating disk usage: " + std::string(e.what()));
        return 0.0;
    }
}

std::optional<std::vector<long long>> SystemMonitor::getSystemStats() {
    std::ifstream statFile("/proc/stat");
    std::string line;
    if (std::getline(statFile, line)) {
        std::istringstream iss(line);
        std::string cpu;
        std::vector<long long> stats;
        long long value;
        iss >> cpu;
        while (iss >> value) {
            stats.push_back(value);
        }
        return stats;
    }
    return std::nullopt;
}

void SystemMonitor::checkAlerts() {
    alertTriggered = cpuUsage > config.getCpuThreshold() ||
                     memoryUsage > config.getMemoryThreshold() ||
                     diskUsage > config.getDiskThreshold();

    logger->log(LogLevel::DEBUG, "CPU Usage: " + std::to_string(cpuUsage) + "%, Threshold: " + std::to_string(config.getCpuThreshold()) + "%");
    logger->log(LogLevel::DEBUG, "Memory Usage: " + std::to_string(memoryUsage) + "%, Threshold: " + std::to_string(config.getMemoryThreshold()) + "%");
    logger->log(LogLevel::DEBUG, "Disk Usage: " + std::to_string(diskUsage) + "%, Threshold: " + std::to_string(config.getDiskThreshold()) + "%");

    if (alertTriggered) {
        std::string alertMessage = "Alert triggered: CPU=" + std::to_string(cpuUsage) + 
                                   "%, Memory=" + std::to_string(memoryUsage) + 
                                   "%, Disk=" + std::to_string(diskUsage) + "%";
        logger->log(LogLevel::WARNING, alertMessage);
        display.showAlert(alertMessage);
    }

    if (nvml_available) {
        auto gpuInfos = gpuMonitor.getGPUInfo();
        for (const auto& gpu : gpuInfos) {
            logger->log(LogLevel::DEBUG, "GPU " + std::to_string(gpu.index) + " Temperature: " + std::to_string(gpu.temperature) + "°C, Threshold: " + std::to_string(GPU_TEMP_THRESHOLD) + "°C");
            if (gpu.temperature > GPU_TEMP_THRESHOLD) {
                std::string gpuAlertMessage = "GPU " + std::to_string(gpu.index) + 
                                              " temperature alert: " + 
                                              std::to_string(gpu.temperature) + "°C (Threshold: " +
                                              std::to_string(GPU_TEMP_THRESHOLD) + "°C)";
                logger->log(LogLevel::WARNING, gpuAlertMessage);
                display.showAlert(gpuAlertMessage);
                alertTriggered = true;
            }
        }
    }
}

void SystemMonitor::run() {
    while (display.handleInput()) {
        update();
        display.update(*this);
        std::this_thread::sleep_for(std::chrono::seconds(config.getUpdateInterval()));
    }
}