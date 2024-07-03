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

SystemMonitor::SystemMonitor(const Config& config, std::shared_ptr<Logger> logger, Display& display)
    : cpuUsage(0), memoryUsage(0), diskUsage(0), alertTriggered(false), 
      config(config), logger(logger), display(display) {}

bool SystemMonitor::initialize() {
    if (!gpuMonitor.initialize()) {
        logger->log(LogLevel::ERROR, "Failed to initialize GPU monitor");
        return false;
    }
    return true;
}

void SystemMonitor::update() {
    cpuUsage = calculateCpuUsage();
    memoryUsage = calculateMemoryUsage();
    diskUsage = calculateDiskUsage();
    processMonitor.update();
    gpuMonitor.update();
    checkAlerts();

    std::string logMessage = "System update: CPU=" + std::to_string(cpuUsage) + 
                             "%, Memory=" + std::to_string(memoryUsage) + 
                             "%, Disk=" + std::to_string(diskUsage) + "%";
    logger->log(LogLevel::INFO, logMessage);
    display.addLogMessage(logMessage);

    auto processes = processMonitor.getProcesses();
    std::sort(processes.begin(), processes.end(), 
              [](const ProcessInfo& a, const ProcessInfo& b) { return a.cpuUsage > b.cpuUsage; });

    for (size_t i = 0; i < std::min<size_t>(5, processes.size()); ++i) {
        const auto& p = processes[i];
        std::string processLog = "Top Process " + std::to_string(i+1) + ": " + p.name + 
                                 " (PID: " + std::to_string(p.pid) + ") CPU: " + 
                                 std::to_string(p.cpuUsage) + "%, Mem: " + 
                                 std::to_string(p.memoryUsage) + " MB";
        logger->log(LogLevel::INFO, processLog);
        display.addLogMessage(processLog);
    }

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
    return gpuMonitor.getGPUInfo();
}

bool SystemMonitor::isAlertTriggered() const {
    return alertTriggered;
}

double SystemMonitor::calculateCpuUsage() {
    static auto lastStats = getSystemStats();
    auto currentStats = getSystemStats();

    if (!lastStats || !currentStats || lastStats->size() < 4 || currentStats->size() < 4) {
        return 0.0;
    }

    auto lastIdle = (*lastStats)[3];
    auto lastTotal = std::accumulate(lastStats->begin(), lastStats->begin() + 4, 0LL);

    auto currentIdle = (*currentStats)[3];
    auto currentTotal = std::accumulate(currentStats->begin(), currentStats->begin() + 4, 0LL);

    auto idleDelta = currentIdle - lastIdle;
    auto totalDelta = currentTotal - lastTotal;

    lastStats = currentStats;

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
    auto space = std::filesystem::space("/");
    auto totalSpace = space.capacity;
    auto freeSpace = space.free;
    auto usedSpace = totalSpace - freeSpace;

    return 100.0 * static_cast<double>(usedSpace) / totalSpace;
}

std::optional<std::vector<long long>> SystemMonitor::getSystemStats() {
    std::ifstream statFile("/proc/stat");
    std::string line;
    if (std::getline(statFile, line)) {
        std::istringstream iss(line);
        std::string cpu;
        long long user, nice, system, idle;
        if (iss >> cpu >> user >> nice >> system >> idle) {
            return std::vector<long long>{user, nice, system, idle};
        }
    }
    return std::nullopt;
}

void SystemMonitor::checkAlerts() {
    alertTriggered = cpuUsage > config.getCpuThreshold() ||
                     memoryUsage > config.getMemoryThreshold() ||
                     diskUsage > config.getDiskThreshold();

    if (alertTriggered) {
        std::string alertMessage = "Alert triggered: CPU=" + std::to_string(cpuUsage) + 
                                   "%, Memory=" + std::to_string(memoryUsage) + 
                                   "%, Disk=" + std::to_string(diskUsage) + "%";
        logger->log(LogLevel::WARNING, alertMessage);
        display.addLogMessage(alertMessage);
    }
}
