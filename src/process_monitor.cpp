#include "../include/process_monitor.h"
#include <algorithm>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

ProcessMonitor::ProcessMonitor() {
    lastUpdateTime = std::chrono::steady_clock::now();
}

ProcessMonitor::~ProcessMonitor() {
    // Empty destructor
}

void ProcessMonitor::update() {
    processes.clear();

    DIR* proc_dir = opendir("/proc");
    if (proc_dir == nullptr) {
        std::cerr << "Failed to open /proc directory: " << strerror(errno) << std::endl;
        return;
    }

    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            std::string filename(entry->d_name);
            if (std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                int pid = std::stoi(filename);
                try {
                    processes.push_back(readProcessInfoFromProc(pid));
                    count++;
                } catch (const std::exception& e) {
                    // Silently ignore errors for individual processes
                }
            }
        }
    }

    closedir(proc_dir);

    if (processes.empty()) {
        std::cerr << "No processes found. This is unexpected." << std::endl;
        std::cerr << "Current user ID: " << getuid() << ", effective user ID: " << geteuid() << std::endl;
    } else {
        std::sort(processes.begin(), processes.end(),
                  [](const ProcessInfo& a, const ProcessInfo& b) { return a.overallUsage > b.overallUsage; });
    }

    auto currentTime = std::chrono::steady_clock::now();
    lastUpdateTime = currentTime;
}

std::vector<ProcessInfo> ProcessMonitor::getProcesses() const {
    return processes;
}

ProcessInfo ProcessMonitor::readProcessInfoFromProc(int pid) {
    ProcessInfo info;
    info.pid = pid;

    try {
        // Read process name
        std::ifstream cmdline("/proc/" + std::to_string(pid) + "/cmdline");
        std::getline(cmdline, info.name, '\0');
        if (info.name.empty()) {
            std::ifstream comm("/proc/" + std::to_string(pid) + "/comm");
            std::getline(comm, info.name);
        }

        // Read CPU usage
        std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
        std::string line;
        std::getline(stat, line);
        std::istringstream iss(line);
        std::string unused;
        unsigned long long utime, stime;
        for (int i = 1; i <= 13; ++i) iss >> unused;
        iss >> utime >> stime;
        unsigned long long totalTime = utime + stime;
        
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastUpdateTime;
        double totalCpuTime = sysconf(_SC_CLK_TCK) * elapsed.count();
        
        // Get the number of logical processors
        long numProcessors = sysconf(_SC_NPROCESSORS_ONLN);
        
        // Calculate CPU usage as a percentage of one core
        info.cpuUsage = (totalCpuTime > 0) ? ((totalTime / totalCpuTime) * 100.0) / numProcessors : 0.0;

        // Read memory usage
        std::ifstream statm("/proc/" + std::to_string(pid) + "/statm");
        unsigned long resident;
        statm >> unused >> resident;
        info.memoryUsage = (resident * sysconf(_SC_PAGESIZE)) / (1024.0 * 1024.0); // Convert to MB

        // Read disk I/O
        std::ifstream io("/proc/" + std::to_string(pid) + "/io");
        while (std::getline(io, line)) {
            std::istringstream iss(line);
            std::string key;
            long long value;
            if (iss >> key >> value) {
                if (key == "read_bytes:") info.diskRead = value;
                else if (key == "write_bytes:") info.diskWrite = value;
            }
        }

        // Calculate overall usage (you can adjust weights as needed)
        info.overallUsage = 0.4 * info.cpuUsage + 0.4 * info.memoryUsage + 
                            0.2 * ((info.diskRead + info.diskWrite) / (1024.0 * 1024.0)); // Convert to MB
    } catch (const std::exception& e) {
        std::cerr << "Error reading info for PID " << pid << ": " << e.what() << std::endl;
    }

    return info;
}