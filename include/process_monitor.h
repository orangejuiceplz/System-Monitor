#pragma once

#include <string>
#include <vector>
#include <filesystem>

struct ProcessInfo {
    int pid;
    std::string name;
    double cpuUsage;
    double memoryUsage;
    long long diskRead;
    long long diskWrite;

    bool operator==(const ProcessInfo& other) const {
        return pid == other.pid &&
               name == other.name &&
               cpuUsage == other.cpuUsage &&
               memoryUsage == other.memoryUsage &&
               diskRead == other.diskRead &&
               diskWrite == other.diskWrite;
    }
};


class ProcessMonitor {
public:
    ProcessMonitor();
    void update();
    std::vector<ProcessInfo> getProcesses() const;

private:
    std::vector<ProcessInfo> processes;
    ProcessInfo getProcessInfo(const std::filesystem::path& procPath);
    double calculateCpuUsage(int pid, unsigned long long& prevCpuTime, unsigned long long& prevSysTime);
};
