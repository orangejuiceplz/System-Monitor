#include "../include/system_monitor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

int main() {
    SystemMonitor monitor;

    while (true) {
        monitor.update();

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "CPU Usage: " << monitor.getCpuUsage() << "%\n";
        std::cout << "Memory Usage: " << monitor.getMemoryUsage() << "%\n";
        std::cout << "Disk Usage: " << monitor.getDiskUsage() << "%\n";
        std::cout << "------------------------\n";

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
