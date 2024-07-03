#include "../include/system_monitor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <algorithm>

int main() {
    SystemMonitor monitor;

    while (true) {
        monitor.update();

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "CPU Usage: " << monitor.getCpuUsage() << "%\n";
        std::cout << "Memory Usage: " << monitor.getMemoryUsage() << "%\n";
        std::cout << "Disk Usage: " << monitor.getDiskUsage() << "%\n";
        std::cout << "------------------------\n";

        std::cout << "Top 5 Processes by CPU Usage:\n";
        auto processes = monitor.getProcesses();
        std::sort(processes.begin(), processes.end(), 
                  [](const ProcessInfo& a, const ProcessInfo& b) { return a.cpuUsage > b.cpuUsage; });
        for (int i = 0; i < std::min(5, static_cast<int>(processes.size())); ++i) {
            const auto& p = processes[i];
            std::cout << p.name << " (PID: " << p.pid << "): CPU " << p.cpuUsage 
                      << "%, Mem " << p.memoryUsage << " MB\n";
        }
        std::cout << "------------------------\n";

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
