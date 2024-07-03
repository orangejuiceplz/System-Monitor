#include "../include/process_monitor.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <unistd.h> // For sysconf

ProcessMonitor::ProcessMonitor() {}

void ProcessMonitor::update() {
    processes.clear();
    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (entry.is_directory() && std::all_of(entry.path().filename().string().begin(), 
                                                entry.path().filename().string().end(), 
                                                ::isdigit)) {
            processes.push_back(getProcessInfo(entry.path()));
        }
    }
}

std::vector<ProcessInfo> ProcessMonitor::getProcesses() const {
    return processes;
}

ProcessInfo ProcessMonitor::getProcessInfo(const std::filesystem::path& procPath) {
    ProcessInfo info;
    info.pid = std::stoi(procPath.filename().string());

    // Read process name
    std::ifstream commFile(procPath / "comm");
    std::getline(commFile, info.name);

    // Read CPU usage
    static std::unordered_map<int, std::pair<unsigned long long, unsigned long long>> prevTimes;
    unsigned long long cpuTime, sysTime;
    info.cpuUsage = calculateCpuUsage(info.pid, cpuTime, sysTime);
    prevTimes[info.pid] = {cpuTime, sysTime};

    // Read memory usage
    std::ifstream statmFile(procPath / "statm");
    long long vmSize, rss;
    statmFile >> vmSize >> rss;
    long pageSize = sysconf(_SC_PAGE_SIZE);
    info.memoryUsage = static_cast<double>(rss * pageSize) / (1024 * 1024); // Convert to MB

    // Read disk I/O
    std::ifstream ioFile(procPath / "io");
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

    return info;
}

double ProcessMonitor::calculateCpuUsage(int pid, unsigned long long& cpuTime, unsigned long long& sysTime) {
    std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat");
    std::string line;
    std::getline(statFile, line);
    std::istringstream iss(line);

    std::vector<std::string> tokens{std::istream_iterator<std::string>(iss),
                                    std::istream_iterator<std::string>()};

    cpuTime = std::stoull(tokens[13]) + std::stoull(tokens[14]);
    sysTime = std::stoull(tokens[21]);

    static std::unordered_map<int, std::pair<unsigned long long, unsigned long long>> prevTimes;
    auto it = prevTimes.find(pid);
    if (it != prevTimes.end()) {
        auto [prevCpuTime, prevSysTime] = it->second;
        auto cpuDelta = cpuTime - prevCpuTime;
        auto sysDelta = sysTime - prevSysTime;
        double cpuUsage = 100.0 * cpuDelta / (cpuDelta + sysDelta);
        return cpuUsage;
    }

    return 0.0;
}
