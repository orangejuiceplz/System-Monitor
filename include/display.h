#pragma once

#include "system_monitor.h"
#include "network_monitor.h"
#include <ncurses.h>
#include <vector>
#include <string>

class Display {
public:
    Display();
    ~Display();
    void update(const SystemMonitor& monitor);
    bool handleInput();
    void forceUpdate(const SystemMonitor& monitor);
    void showAlert(const std::string& message);
    void addLogMessage(const std::string& message); 

private:
    WINDOW* mainWindow;
    WINDOW* cpuWindow;
    WINDOW* memoryWindow;
    WINDOW* diskWindow;
    WINDOW* logWindow;
    WINDOW* processWindow;
    WINDOW* networkWindow;
    WINDOW* batteryWindow;
    WINDOW* gpuWindow;
    WINDOW* timeWindow;

    std::vector<std::string> logMessages;
    size_t processListScrollPosition;
    bool needsUpdate;
    int networkWindowWidth;

    void initializeScreen();
    void updateMainWindow(const SystemMonitor& monitor);
    void updateCPUWindow(const SystemMonitor& monitor);
    void updateMemoryWindow(const SystemMonitor& monitor);
    void updateDiskWindow(const SystemMonitor& monitor);
    void updateProcessWindow(const std::vector<ProcessInfo>& processes);
    void updateNetworkInfo(const std::vector<NetworkInterface>& interfaces);
    void updateLogWindow();
    void updateGPUInfo(const std::vector<GPUInfo>& gpuInfos);
    void updateBatteryInfo(const SystemMonitor& monitor);
    void updateTimeInfo(const SystemMonitor& monitor);
    void scrollProcessList(int direction);

    std::string formatUptime(long uptime) const;
    std::string getCurrentTime() const;

    static const size_t MAX_LOG_MESSAGES = 10;
    static const int PROCESS_WINDOW_HEIGHT = 15;
};
