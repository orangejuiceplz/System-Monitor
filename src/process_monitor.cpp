#include "../include/process_monitor.h"
#include <algorithm>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

ProcessMonitor::ProcessMonitor() : proc(nullptr) {
    lastUpdateTime = std::chrono::steady_clock::now();
}

ProcessMonitor::~ProcessMonitor() {
    if (proc) {
        closeproc(proc);
    }
}

void ProcessMonitor::update() {
    processes.clear();

    if (!proc) {
        proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
    }

    if (!proc) {
        throw std::runtime_error("Failed to open proc");
    }

    proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_info));

    while (readproc(proc, &proc_info) != nullptr) {
        processes.push_back(getProcessInfo(&proc_info));
        memset(&proc_info, 0, sizeof(proc_info));
    }

    std::cout << "Found " << processes.size() << " processes" << std::endl;

    if (processes.empty()) {
        std::cerr << "No processes found. Check permissions and /proc filesystem access." << std::endl;
    } else {
        // Sort processes by overall usage
        std::sort(processes.begin(), processes.end(),
                  [](const ProcessInfo& a, const ProcessInfo& b) { return a.overallUsage > b.overallUsage; });
    }

    auto currentTime = std::chrono::steady_clock::now();
    lastUpdateTime = currentTime;
}

std::vector<ProcessInfo> ProcessMonitor::getProcesses() const {
    return processes;
}

ProcessInfo ProcessMonitor::getProcessInfo(proc_t* proc_info) {
    ProcessInfo info;
    info.pid = proc_info->tid;
    info.name = proc_info->cmd;
    
    unsigned long long totalTime = proc_info->utime + proc_info->stime;
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = currentTime - lastUpdateTime;
    
    double totalCpuTime = sysconf(_SC_CLK_TCK) * elapsed.count();
    info.cpuUsage = (totalTime / totalCpuTime) * 100.0;
    info.memoryUsage = (proc_info->resident * sysconf(_SC_PAGESIZE)) / (1024.0 * 1024.0); // Convert to MB
    
    // Read disk I/O information from /proc/<pid>/io
    std::ifstream ioFile("/proc/" + std::to_string(info.pid) + "/io");
    std::string line;
    while (std::getline(ioFile, line)) {
        std::istringstream iss(line);
        std::string key;
        long long value;
        if (iss >> key >> value) {
            if (key == "read_bytes:") info.diskRead = value;
            else if (key == "write_bytes:") info.diskWrite = value;
        }
    }

    // Calculate overall usage (you can adjust weights as needed)
      // Calculate overall usage (you can adjust weights as needed)
    info.overallUsage = 0.4 * info.cpuUsage + 0.4 * info.memoryUsage + 
                        0.2 * ((info.diskRead + info.diskWrite) / (1024.0 * 1024.0)); // Convert to MB

    return info;
}

double ProcessMonitor::calculateCpuUsage(unsigned long long lastCpuTime, unsigned long long currentCpuTime) {
    unsigned long long cpuTimeDiff = currentCpuTime - lastCpuTime;
    return (cpuTimeDiff / sysconf(_SC_CLK_TCK)) * 100.0;
}