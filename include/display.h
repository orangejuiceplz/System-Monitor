#pragma once

#include "system_monitor.h"
#include <ncurses.h>
#include <vector>
#include <string>

class Display {
public:
    Display();
    ~Display();

    void update(const SystemMonitor& monitor);
    bool handleInput();
    void showAlert(const std::string& message);
    void addLogMessage(const std::string& message);

private:
    void initializeScreen();
    void updateMainWindow(const SystemMonitor& monitor);
    void updateLogWindow();
    void updateGPUInfo(const std::vector<GPUInfo>& gpuInfos);
    void updateProcessList(const std::vector<ProcessInfo>& processes);
    void displayProcessList();

    WINDOW* mainWindow;
    WINDOW* logWindow;
    WINDOW* processListPad;
    int processListScrollPosition;
    std::vector<std::string> logMessages;
    static const size_t MAX_LOG_MESSAGES = 10;
};
