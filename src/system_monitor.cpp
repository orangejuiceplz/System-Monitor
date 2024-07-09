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

bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           std::equal(prefix.begin(), prefix.end(), str.begin());
}

const float SystemMonitor::GPU_TEMP_THRESHOLD = 80.0f;

SystemMonitor::SystemMonitor(const Config& config, std::shared_ptr<Logger> logger, Display& display, bool nvml_available)
    : cpuUsage(0), memoryUsage(0), diskUsage(0), alertTriggered(false), 
      nvml_available(nvml_available), config(config), logger(logger), display(display),
      processMonitorThread(config.getUpdateIntervalMs()), gpuUnavailabilityLogged(false),
      totalMemory(0), totalDiskSpace(0), uptime(0) {}

bool SystemMonitor::initialize() {
    if (!initializeGPU()) {
        nvml_available = false;
    }
    initializeCpuInfo();
    initializeMemoryInfo();
    initializeDiskInfo();
    processMonitorThread.start();
    return true;
}

void SystemMonitor::initializeCpuInfo() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    int coreCount = 0;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            cpuModel = line.substr(line.find(":") + 2);
        }
        if (line.find("processor") != std::string::npos) {
            coreCount++;
        }
    }
    cpuCoreInfo.resize(coreCount);
}

void SystemMonitor::initializeMemoryInfo() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") != std::string::npos) {
            std::istringstream iss(line);
            std::string key;
            unsigned long long value;
            iss >> key >> value;
            totalMemory = value * 1024; // Convert from KB to bytes
            break;
        }
    }
}

void SystemMonitor::initializeDiskInfo() {
    try {
        std::string rootDevice = getRootDeviceName();
        if (!rootDevice.empty()) {
            diskName = rootDevice;
            auto space = std::filesystem::space("/");
            totalDiskSpace = space.capacity;
        } else {
            logger->logError("No suitable drive found");
        }
    } catch (const std::filesystem::filesystem_error& e) {
        logger->logError("Error getting disk info: " + std::string(e.what()));
    }
    updateDiskPartitions();
}

std::string SystemMonitor::getRootDeviceName() {
    std::ifstream mounts("/proc/mounts");
    std::string line;
    while (std::getline(mounts, line)) {
        std::istringstream iss(line);
        std::string device, mountpoint, fs;
        iss >> device >> mountpoint >> fs;
        if (mountpoint == "/") {
            return device;
        }
    }
    return "";
}

bool SystemMonitor::initializeGPU() {
    if (nvml_available) {
        try {
            if (!gpuMonitor.initialize()) {
                logger->logError("Failed to initialize GPU monitor");
                display.addLogMessage("GPU monitoring unavailable: Failed to initialize");
                gpuUnavailabilityLogged = true;
                return false;
            }
        } catch (const std::exception& e) {
            logger->logError("Exception during GPU monitor initialization: " + std::string(e.what()));
            display.addLogMessage("GPU monitoring unavailable: " + std::string(e.what()));
            gpuUnavailabilityLogged = true;
            return false;
        } catch (...) {
            logger->logError("Unknown exception during GPU monitor initialization");
            display.addLogMessage("GPU monitoring unavailable: Unknown error");
            gpuUnavailabilityLogged = true;
            return false;
        }
    } else if (!gpuUnavailabilityLogged) {
        logger->logWarning("GPU monitoring is not available");
        display.addLogMessage("GPU monitoring not available");
        gpuUnavailabilityLogged = true;
    }
    return nvml_available;
}

void SystemMonitor::update() {
    cpuUsage = calculateCpuUsage();
    updateCPUCoreInfo();
    memoryUsage = calculateMemoryUsage();
    diskUsage = calculateDiskUsage();
    updateDiskPartitions();
    if (nvml_available) {
        gpuMonitor.update();
    }
    networkMonitor.update();
    batteryMonitor.update();
    updateUptime();
    checkAlerts();
}

double SystemMonitor::getCpuUsage() const {
    return cpuUsage;
}

const std::vector<CPUCoreInfo>& SystemMonitor::getCPUCoreInfo() const {
    return cpuCoreInfo;
}

double SystemMonitor::getMemoryUsage() const {
    return memoryUsage;
}

double SystemMonitor::getDiskUsage() const {
    return diskUsage;
}

const std::vector<DiskPartitionInfo>& SystemMonitor::getDiskPartitions() const {
    return diskPartitions;
}

std::vector<ProcessInfo> SystemMonitor::getProcesses() const {
    return processMonitorThread.getProcesses();
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

std::string SystemMonitor::getCpuModel() const {
    return cpuModel;
}

unsigned long long SystemMonitor::getTotalMemory() const {
    return totalMemory;
}

unsigned long long SystemMonitor::getTotalDiskSpace() const {
    return totalDiskSpace;
}

std::string SystemMonitor::getDiskName() const {
    return diskName;
}

const BatteryMonitor& SystemMonitor::getBatteryMonitor() const {
    return batteryMonitor;
}

long SystemMonitor::getUptime() const {
    return uptime;
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

void SystemMonitor::updateCPUCoreInfo() {
    std::ifstream statFile("/proc/stat");
    std::string line;
    int coreIndex = 0;

    while (std::getline(statFile, line) && coreIndex < cpuCoreInfo.size()) {
        if (line.substr(0, 3) == "cpu" && line[3] != ' ') {
            std::istringstream ss(line);
            std::string cpu;
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
            
            ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

            unsigned long long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
            unsigned long long idleTime = idle + iowait;

            static std::vector<unsigned long long> lastTotalTime(cpuCoreInfo.size(), 0);
            static std::vector<unsigned long long> lastIdleTime(cpuCoreInfo.size(), 0);

            if (lastTotalTime[coreIndex] != 0) {
                unsigned long long totalTimeDiff = totalTime - lastTotalTime[coreIndex];
                unsigned long long idleTimeDiff = idleTime - lastIdleTime[coreIndex];
                cpuCoreInfo[coreIndex].utilization = 100.0 * (1.0 - static_cast<double>(idleTimeDiff) / totalTimeDiff);
            }

            lastTotalTime[coreIndex] = totalTime;
            lastIdleTime[coreIndex] = idleTime;

            // Read core temperature (this might need to be adjusted based on your system)
            std::ifstream tempFile("/sys/class/thermal/thermal_zone" + std::to_string(coreIndex) + "/temp");
            int temp;
            if (tempFile >> temp) {
                cpuCoreInfo[coreIndex].temperature = temp / 1000.0; // Convert from millidegrees to degrees
            }

            // Read core clock speed
            std::ifstream freqFile("/sys/devices/system/cpu/cpu" + std::to_string(coreIndex) + "/cpufreq/scaling_cur_freq");
            unsigned long long freq;
            if (freqFile >> freq) {
                cpuCoreInfo[coreIndex].clockSpeed = freq / 1000000.0; // Convert from kHz to GHz
            }

            coreIndex++;
        }
    }
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

void SystemMonitor::updateDiskPartitions() {
    diskPartitions.clear();
    std::ifstream mountsFile("/proc/mounts");
    std::string line;

    while (std::getline(mountsFile, line)) {
        std::istringstream iss(line);
        std::string device, mountPoint, fsType;
        iss >> device >> mountPoint >> fsType;

        if (starts_with(device, "/dev/") && fsType != "tmpfs" && fsType != "devtmpfs") {
            try {
                auto space = std::filesystem::space(mountPoint);
                DiskPartitionInfo partInfo;
                partInfo.name = device.substr(5); // Remove "/dev/" prefix
                partInfo.mountPoint = mountPoint;
                partInfo.totalSpace = space.capacity;
                partInfo.usedSpace = space.capacity - space.free;
                diskPartitions.push_back(partInfo);
            } catch (const std::filesystem::filesystem_error& e) {
                logger->logError("Error getting partition info for " + device + ": " + e.what());
            }
        }
    }
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

std::vector<NetworkInterface> SystemMonitor::getNetworkInterfaces() const {
    return networkMonitor.getActiveInterfaces();
}

void SystemMonitor::updateUptime() {
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        uptime = si.uptime;
    }
}

void SystemMonitor::run() {
    while (true) {
        update();
        display.update(*this);
        
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start).count() < config.getUpdateIntervalMs()) {
            if (!display.handleInput()) {
                processMonitorThread.stop();
                return;
            }
            display.forceUpdate(*this);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}
