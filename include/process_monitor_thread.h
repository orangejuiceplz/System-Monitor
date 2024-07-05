#pragma once

#include "process_monitor.h"
#include <thread>
#include <mutex>
#include <atomic>

class ProcessMonitorThread {
public:
    ProcessMonitorThread(int updateInterval);
    ~ProcessMonitorThread();
    void start();
    void stop();
    std::vector<ProcessInfo> getProcesses() const;

private:
    void run();
    mutable ProcessMonitor processMonitor;
    std::thread monitorThread;
    mutable std::mutex processesMutex;
    std::atomic<bool> running;
    int updateInterval;
};