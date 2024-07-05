#include "../include/process_monitor_thread.h"
#include <chrono>

ProcessMonitorThread::ProcessMonitorThread(int updateInterval)
    : running(false), updateInterval(updateInterval) {}

ProcessMonitorThread::~ProcessMonitorThread() {
    stop();
}

void ProcessMonitorThread::start() {
    running = true;
    monitorThread = std::thread(&ProcessMonitorThread::run, this);
}

void ProcessMonitorThread::stop() {
    running = false;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}

std::vector<ProcessInfo> ProcessMonitorThread::getProcesses() const {
    std::lock_guard<std::mutex> lock(processesMutex);
    return processMonitor.getProcesses();
}

void ProcessMonitorThread::run() {
    while (running) {
        {
            std::lock_guard<std::mutex> lock(processesMutex);
            processMonitor.update();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(updateInterval));
    }
}
