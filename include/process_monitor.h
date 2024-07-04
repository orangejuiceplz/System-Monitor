#pragma once

#include <vector>
#include <string>
#include <chrono>

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
};
