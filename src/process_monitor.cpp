#include "../include/process_monitor.h"
#include <algorithm>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <map>
#include <filesystem>

ProcessMonitor::ProcessMonitor() {
    lastUpdateTime = std::chrono::steady_clock::now();
}

ProcessMonitor::~ProcessMonitor() {
    // destructor
}

void ProcessMonitor::update() {
    auto currentTime = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration<double>(currentTime - lastUpdateTime).count();
    
    std::vector<ProcessInfo> newProcesses;
    static std::map<int, unsigned long long> lastTotalTimes;

    DIR* proc_dir = opendir("/proc");
    if (proc_dir == nullptr) {
        std::cerr << "Failed to open /proc directory: " << strerror(errno) << std::endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            std::string filename(entry->d_name);
            if (std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                int pid = std::stoi(filename);
                try {
                    ProcessInfo info = readProcessInfoFromProc(pid);
                    info.cpuUsage = calculateCPUUsage(pid);
                    newProcesses.push_back(info);
                } catch (const std::exception& e) {
                    // silently ignore error
                }
            }
        }
    }

    closedir(proc_dir);

    double totalSystemMemory = getTotalSystemMemory();

    for (auto& process : newProcesses) {
        double cpuPercentage = process.cpuUsage / sysconf(_SC_NPROCESSORS_ONLN);
        double memoryPercentage = (totalSystemMemory > 0) ? (process.memoryUsage / totalSystemMemory) * 100.0 : 0.0;
        process.overallUsage = (cpuPercentage + memoryPercentage) / 2.0;
    }

    std::sort(newProcesses.begin(), newProcesses.end(),
              [](const ProcessInfo& a, const ProcessInfo& b) { return a.overallUsage > b.overallUsage; });

    processes = std::move(newProcesses);
    lastUpdateTime = currentTime;
}

std::vector<ProcessInfo> ProcessMonitor::getProcesses() const {
    return processes;
}

ProcessInfo ProcessMonitor::readProcessInfoFromProc(int pid) {
    ProcessInfo info;
    info.pid = pid;

    try {
        std::string comm;
        {
            std::ifstream comm_file("/proc/" + std::to_string(pid) + "/comm");
            std::getline(comm_file, comm);
        }
        
        if (!comm.empty()) {
            info.name = comm;
        } else {
            std::string cmdline;
            {
                std::ifstream cmdline_file("/proc/" + std::to_string(pid) + "/cmdline");
                std::getline(cmdline_file, cmdline, '\0');
            }
            
            if (!cmdline.empty()) {
                std::filesystem::path p(cmdline.substr(0, cmdline.find(' ')));
                info.name = p.filename().string();
            } else {
                info.name = "unknown";
            }
        }


        info.name.erase(std::remove_if(info.name.begin(), info.name.end(), 
                                       [](unsigned char c) { return !std::isprint(c); }),
                        info.name.end());
        if (info.name.length() > MAX_NAME_LENGTH) {
            info.name = info.name.substr(0, TRUNCATE_LENGTH) + "...";
        }

        unsigned long long vm_size, vm_rss;
        {
            std::ifstream statm_file("/proc/" + std::to_string(pid) + "/statm");
            statm_file >> vm_size >> vm_rss;
        }
        info.memoryUsage = (vm_rss * sysconf(_SC_PAGESIZE)) / (1024.0 * 1024.0);

        {
            std::ifstream io_file("/proc/" + std::to_string(pid) + "/io");
            std::string line;
            while (std::getline(io_file, line)) {
                std::istringstream iss(line);
                std::string key;
                long long value;
                if (iss >> key >> value) {
                    if (key == "read_bytes:") info.diskRead = value;
                    else if (key == "write_bytes:") info.diskWrite = value;
                }
            }
        }

        info.cpuUsage = calculateCPUUsage(pid);

        info.overallUsage = CPU_WEIGHT * info.cpuUsage + 
                            MEMORY_WEIGHT * info.memoryUsage + 
                            DISK_WEIGHT * ((info.diskRead + info.diskWrite) / (1024.0 * 1024.0));

    } catch (const std::exception& e) {
        std::cerr << "Error reading info for PID " << pid << ": " << e.what() << std::endl;
        info.name = "error";
        info.memoryUsage = 0;
        info.diskRead = 0;
        info.diskWrite = 0;
        info.cpuUsage = 0;
        info.overallUsage = 0;
    }

    return info;
}

double ProcessMonitor::getTotalSystemMemory() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.compare(0, 9, "MemTotal:") == 0) {
            long long memTotal;
            std::istringstream iss(line);
            std::string dummy;
            iss >> dummy >> memTotal;
            return memTotal * 1024.0; 
        }
    }
    return 0.0;
}

double ProcessMonitor::calculateCPUUsage(int pid) {
    static std::map<int, std::pair<unsigned long long, std::chrono::steady_clock::time_point>> lastValues;

    try {
        std::ifstream stat_file("/proc/" + std::to_string(pid) + "/stat");
        std::string line;
        std::getline(stat_file, line);
        std::istringstream iss(line);

        std::string unused;
        unsigned long long utime, stime;
        for (int i = 1; i <= 13; ++i) iss >> unused;
        iss >> utime >> stime;

        unsigned long long total_time = utime + stime;
        auto current_time = std::chrono::steady_clock::now();

        double cpu_usage = 0.0;
        if (lastValues.find(pid) != lastValues.end()) {
            const auto& [last_total_time, last_time] = lastValues[pid];
            auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();
            
            if (time_diff > 0) {
                unsigned long long time_diff_total = total_time - last_total_time;
                cpu_usage = (time_diff_total * 1000.0 / (sysconf(_SC_CLK_TCK) * time_diff)) * 100.0;
            }
        }

        lastValues[pid] = {total_time, current_time};
        return cpu_usage;

    } catch (const std::exception& e) {
        std::cerr << "Error calculating CPU usage for PID " << pid << ": " << e.what() << std::endl;
        return 0.0;
    }
}