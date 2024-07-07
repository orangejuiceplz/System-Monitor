#include "../include/display.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>

Display::Display() : mainWindow(nullptr), cpuWindow(nullptr), memoryWindow(nullptr), diskWindow(nullptr),
                     logWindow(nullptr), processWindow(nullptr), networkWindow(nullptr), 
                     batteryWindow(nullptr), gpuWindow(nullptr), timeWindow(nullptr),
                     processListScrollPosition(0), needsUpdate(false) {
    initializeScreen();
}

Display::~Display() {
    if (mainWindow) delwin(mainWindow);
    if (cpuWindow) delwin(cpuWindow);
    if (memoryWindow) delwin(memoryWindow);
    if (diskWindow) delwin(diskWindow);
    if (logWindow) delwin(logWindow);
    if (processWindow) delwin(processWindow);
    if (networkWindow) delwin(networkWindow);
    if (batteryWindow) delwin(batteryWindow);
    if (gpuWindow) delwin(gpuWindow);
    if (timeWindow) delwin(timeWindow);
    endwin();
}

void Display::initializeScreen() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();

    if (COLORS >= 8) {
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    }

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    int halfWidth = xMax / 2;
    int processWidth = 2 * halfWidth / 3;
    int networkWidth = halfWidth - processWidth;
    int logWidth = xMax - halfWidth;

    timeWindow = newwin(1, xMax, 0, 0);
    cpuWindow = newwin(8, halfWidth, 1, 0);
    gpuWindow = newwin(8, halfWidth, 1, halfWidth);
    memoryWindow = newwin(6, halfWidth, 9, 0);
    diskWindow = newwin(6, halfWidth, 9, halfWidth);
    processWindow = newwin(yMax - 15, processWidth, 15, 0);
    networkWindow = newwin((yMax - 15) / 2, networkWidth, 15, processWidth);
    batteryWindow = newwin((yMax - 15) / 2, networkWidth, 15 + (yMax - 15) / 2, processWidth);
    logWindow = newwin(yMax - 15, logWidth, 15, halfWidth);

    // Enable keypad input for all windows
    keypad(mainWindow, TRUE);
    keypad(cpuWindow, TRUE);
    keypad(memoryWindow, TRUE);
    keypad(diskWindow, TRUE);
    keypad(logWindow, TRUE);
    keypad(processWindow, TRUE);
    keypad(networkWindow, TRUE);
    keypad(batteryWindow, TRUE);
    keypad(gpuWindow, TRUE);
    keypad(timeWindow, TRUE);
}

void Display::update(const SystemMonitor& monitor) {
    updateTimeInfo(monitor);
    updateCPUWindow(monitor);
    updateGPUInfo(monitor.getGPUInfo());
    updateMemoryWindow(monitor);
    updateDiskWindow(monitor);
    updateProcessWindow(monitor.getProcesses());
    updateNetworkInfo(monitor.getNetworkInterfaces());
    updateBatteryInfo(monitor);
    updateLogWindow();
}

void Display::updateTimeInfo(const SystemMonitor& monitor) {
    wclear(timeWindow);
    mvwprintw(timeWindow, 0, (COLS - 40) / 2, "Current Time: %s | Uptime: %s", 
              getCurrentTime().c_str(), formatUptime(monitor.getUptime()).c_str());
    wrefresh(timeWindow);
}

void Display::updateCPUWindow(const SystemMonitor& monitor) {
    wclear(cpuWindow);
    box(cpuWindow, 0, 0);
    mvwprintw(cpuWindow, 0, 2, "CPU");
    mvwprintw(cpuWindow, 1, 2, "Model: %s", monitor.getCpuModel().c_str());
    mvwprintw(cpuWindow, 2, 2, "Overall Usage: %.2f%%", monitor.getCpuUsage());

    const auto& coreInfo = monitor.getCPUCoreInfo();
    for (size_t i = 0; i < coreInfo.size() && i < 4; ++i) {
        mvwprintw(cpuWindow, 3 + i, 2, "Core %zu: %.2f%% (%.1f°C)", 
                  i, coreInfo[i].utilization, coreInfo[i].temperature);
    }
    wrefresh(cpuWindow);
}

void Display::updateMemoryWindow(const SystemMonitor& monitor) {
    wclear(memoryWindow);
    box(memoryWindow, 0, 0);
    mvwprintw(memoryWindow, 0, 2, "Memory");
    double totalMemoryGB = monitor.getTotalMemory() / (1024.0 * 1024 * 1024);
    mvwprintw(memoryWindow, 1, 2, "Total: %.2f GB", totalMemoryGB);
    mvwprintw(memoryWindow, 2, 2, "Usage: %.2f%%", monitor.getMemoryUsage());
    wrefresh(memoryWindow);
}

void Display::updateDiskWindow(const SystemMonitor& monitor) {
    wclear(diskWindow);
    box(diskWindow, 0, 0);
    mvwprintw(diskWindow, 0, 2, "Disk");
    const auto& partitions = monitor.getDiskPartitions();
    for (size_t i = 0; i < partitions.size() && i < 4; ++i) {
        const auto& part = partitions[i];
        double totalGB = part.totalSpace / (1024.0 * 1024 * 1024);
        double usedGB = part.usedSpace / (1024.0 * 1024 * 1024);
        double usagePercent = (static_cast<double>(part.usedSpace) / part.totalSpace) * 100.0;
        mvwprintw(diskWindow, 1 + i, 2, "%s (%s): %.1f/%.1f GB (%.2f%%)", 
                  part.name.c_str(), part.mountPoint.c_str(), usedGB, totalGB, usagePercent);
    }
    wrefresh(diskWindow);
}

void Display::updateProcessWindow(const std::vector<ProcessInfo>& processes) {
    wclear(processWindow);
    box(processWindow, 0, 0);
    mvwprintw(processWindow, 0, 2, "Process List (Use UP/DOWN to scroll)");
    int maxRows, maxCols;
    getmaxyx(processWindow, maxRows, maxCols);
    int displayableRows = maxRows - 2;  // Subtract 2 for the box borders

    size_t startIndex = processListScrollPosition;
    size_t endIndex = std::min(startIndex + displayableRows, processes.size());

    for (size_t i = startIndex; i < endIndex; ++i) {
        const auto& process = processes[i];
        mvwprintw(processWindow, i - startIndex + 1, 1, "%-20s CPU: %5.1f%% Mem: %5.1f MB",
                  process.name.c_str(), process.cpuUsage, process.memoryUsage);
    }
    wrefresh(processWindow);
}

void Display::updateNetworkInfo(const std::vector<NetworkInterface>& interfaces) {
    wclear(networkWindow);
    box(networkWindow, 0, 0);
    mvwprintw(networkWindow, 0, 2, "Network Information");
    int row = 1;
    double maxDownloadSpeed = 0;
    double maxUploadSpeed = 0;

    for (const auto& interface : interfaces) {
        mvwprintw(networkWindow, row++, 1, "%s (%s)", interface.name.c_str(), interface.type.c_str());
        mvwprintw(networkWindow, row++, 1, "IP: %s", interface.ipAddress.c_str());
        mvwprintw(networkWindow, row++, 1, "Down: %.2f MB/s", interface.downloadSpeed / (1024 * 1024));
        mvwprintw(networkWindow, row++, 1, "Up: %.2f MB/s", interface.uploadSpeed / (1024 * 1024));
        row++;

        maxDownloadSpeed = std::max(maxDownloadSpeed, interface.downloadSpeed);
        maxUploadSpeed = std::max(maxUploadSpeed, interface.uploadSpeed);
    }

    mvwprintw(networkWindow, row++, 1, "Max Down: %.2f MB/s", maxDownloadSpeed / (1024 * 1024));
    mvwprintw(networkWindow, row++, 1, "Max Up: %.2f MB/s", maxUploadSpeed / (1024 * 1024));

    wrefresh(networkWindow);
}


void Display::updateLogWindow() {
    wclear(logWindow);
    box(logWindow, 0, 0);
    mvwprintw(logWindow, 0, 2, "Log Messages");
    size_t startIndex = logMessages.size() > MAX_LOG_MESSAGES ? logMessages.size() - MAX_LOG_MESSAGES : 0;
    for (size_t i = 0; i < MAX_LOG_MESSAGES && startIndex + i < logMessages.size(); ++i) {
        mvwprintw(logWindow, i + 1, 2, "%s", logMessages[startIndex + i].c_str());
    }
    wrefresh(logWindow);
}

void Display::updateGPUInfo(const std::vector<GPUInfo>& gpuInfos) {
    wclear(gpuWindow);
    box(gpuWindow, 0, 0);
    mvwprintw(gpuWindow, 0, 2, "GPU");
    for (size_t i = 0; i < gpuInfos.size() && i < 2; ++i) {
        const auto& gpu = gpuInfos[i];
        mvwprintw(gpuWindow, 1 + i * 2, 2, "GPU %d: %s", gpu.index, gpu.name.c_str());
        mvwprintw(gpuWindow, 2 + i * 2, 2, "Temp: %.1f°C | Util: %.1f%% | Mem: %.1f%%", 
                  gpu.temperature, gpu.gpuUtilization, gpu.memoryUtilization);
    }
    wrefresh(gpuWindow);
}

void Display::updateBatteryInfo(const SystemMonitor& monitor) {
    wclear(batteryWindow);
    box(batteryWindow, 0, 0);
    mvwprintw(batteryWindow, 0, 2, "Battery");
    const auto& battery = monitor.getBatteryMonitor();
    mvwprintw(batteryWindow, 1, 2, "State: %s", battery.getState().c_str());
    mvwprintw(batteryWindow, 2, 2, "Percentage: %.2f%%", battery.getPercentage());
    mvwprintw(batteryWindow, 3, 2, "Est. Time: %s", battery.getEstimatedTime().c_str());
    wrefresh(batteryWindow);
}

void Display::scrollProcessList(int direction) {
    processListScrollPosition += direction;
    if (processListScrollPosition < 0) {
        processListScrollPosition = 0;
    }
    needsUpdate = true;
}

void Display::showAlert(const std::string& message) {
    addLogMessage("ALERT: " + message);
}

void Display::addLogMessage(const std::string& message) {
    std::string timestampedMessage = getCurrentTime() + " - " + message;
    logMessages.push_back(timestampedMessage);
    if (logMessages.size() > MAX_LOG_MESSAGES) {
        logMessages.erase(logMessages.begin());
    }
    updateLogWindow();
}

bool Display::handleInput() {
    int ch = wgetch(stdscr);
    switch (ch) {
        case 'q':
        case 'Q':
            return false;
        case KEY_UP:
            scrollProcessList(-1);
            return true;
        case KEY_DOWN:
            scrollProcessList(1);
            return true;
        default:
            return true;
    }
}

void Display::forceUpdate(const SystemMonitor& monitor) {
    if (needsUpdate) {
        updateProcessWindow(monitor.getProcesses());
        needsUpdate = false;
    }
}

std::string Display::formatUptime(long uptime) const {
    int days = uptime / (24 * 3600);
    int hours = (uptime % (24 * 3600)) / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;

    std::ostringstream oss;
    oss << days << "d " << hours << "h " << minutes << "m " << seconds << "s";
    return oss.str();
}

std::string Display::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
