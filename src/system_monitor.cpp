#include "../include/system_monitor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <numeric>

SystemMonitor::SystemMonitor(const Config& config)
    : cpuUsage(0), memoryUsage(0), diskUsage(0), alertTriggered(false), config(config) {}

void SystemMonitor::update() {
    cpuUsage = calculateCpuUsage();
    memoryUsage = calculateMemoryUsage();
    diskUsage = calculateDiskUsage();
    processMonitor.update();
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
    static const auto rootPath = std::filesystem::path("/");
    auto space = std::filesystem::space(rootPath);
    
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

std::vector<ProcessInfo> SystemMonitor::getProcesses() const {
    return processMonitor.getProcesses();
}

void SystemMonitor::checkAlerts() {
    alertTriggered = cpuUsage > config.getCpuThreshold() ||
                     memoryUsage > config.getMemoryThreshold() ||
                     diskUsage > config.getDiskThreshold();
}

bool SystemMonitor::isAlertTriggered() const {
    return alertTriggered;
}