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
                logger->logError("Failed to initialize GPU monitor");
                nvml_available = false;
            }
        } catch (const std::exception& e) {
            logger->logError("Exception during GPU monitor initialization: " + std::string(e.what()));
            nvml_available = false;
        } catch (...) {
            logger->logError("Unknown exception during GPU monitor initialization");
            nvml_available = false;
        }
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
    static unsigned long long lastTotalUser = 0, lastTotalUserLow = 0, lastTotalSys = 0, lastTotalIdle = 0;

    std::ifstream statFile("/proc/stat");
    std::string line;
    std::getline(statFile, line);
    std::istringstream ss(line);

    std::string cpu;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, totalIOwait, totalIRQ, totalSoftIRQ;

    ss >> cpu >> totalUser >> totalUserLow >> totalSys >> totalIdle >> totalIOwait >> totalIRQ >> totalSoftIRQ;

    if (lastTotalUser == 0) {
        // First call, just set the last values
        lastTotalUser = totalUser;
        lastTotalUserLow = totalUserLow;
        lastTotalSys = totalSys;
        lastTotalIdle = totalIdle;
        return 0.0;
    }

    unsigned long long total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                               (totalSys - lastTotalSys);
    total += (totalIdle - lastTotalIdle);
    double percent = (total - (totalIdle - lastTotalIdle)) / static_cast<double>(total);

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return percent * 100.0;
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
        logger->logError("Error calculating disk usage: " + std::string(e.what()));
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
    bool cpuAlert = cpuUsage > config.getCpuThreshold();
    bool memoryAlert = memoryUsage > config.getMemoryThreshold();
    bool diskAlert = diskUsage > config.getDiskThreshold();

    if (cpuAlert || memoryAlert || diskAlert) {
        std::string alertMessage = "Alert triggered:";
        if (cpuAlert) alertMessage += " CPU=" + std::to_string(cpuUsage) + "%";
        if (memoryAlert) alertMessage += " Memory=" + std::to_string(memoryUsage) + "%";
        if (diskAlert) alertMessage += " Disk=" + std::to_string(diskUsage) + "%";
        
        logger->logWarning(alertMessage);
        display.showAlert(alertMessage);
        alertTriggered = true;
    } else {
        alertTriggered = false;
    }

    if (nvml_available) {
        auto gpuInfos = gpuMonitor.getGPUInfo();
        for (const auto& gpu : gpuInfos) {
            if (gpu.temperature > GPU_TEMP_THRESHOLD) {
                std::string gpuAlertMessage = "GPU " + std::to_string(gpu.index) + 
                                              " temperature alert: " + 
                                              std::to_string(gpu.temperature) + "°C (Threshold: " +
                                              std::to_string(GPU_TEMP_THRESHOLD) + "°C)";
                logger->logWarning(gpuAlertMessage);
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
