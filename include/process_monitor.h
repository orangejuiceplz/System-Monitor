#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <proc/readproc.h>

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
    PROCTAB* proc;

    ProcessInfo getProcessInfo(proc_t* proc_info);
    double calculateCpuUsage(unsigned long long lastCpuTime, unsigned long long currentCpuTime);
};
