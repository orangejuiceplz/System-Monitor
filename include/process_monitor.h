#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <map>

struct ProcessInfo {
    int pid;
    std::string name;
    double cpuUsage;
    double memoryUsage;
    long long diskRead;
    long long diskWrite;
    double overallUsage;
};

class ProcessMonitor {
public:
    ProcessMonitor();
    ~ProcessMonitor();
    void update();
    std::vector<ProcessInfo> getProcesses() const;

private:
    std::vector<ProcessInfo> processes;
    std::chrono::steady_clock::time_point lastUpdateTime;
    ProcessInfo readProcessInfoFromProc(int pid);
    double calculateCPUUsage(int pid);
    double getTotalSystemMemory();
    std::map<int, std::pair<unsigned long long, std::chrono::steady_clock::time_point>> lastCPUTimes;
    static constexpr double CPU_WEIGHT = 0.4;
    static constexpr double MEMORY_WEIGHT = 0.4;
    static constexpr double DISK_WEIGHT = 0.2;
    static constexpr size_t MAX_NAME_LENGTH = 15;
    static constexpr size_t TRUNCATE_LENGTH = 12;
};