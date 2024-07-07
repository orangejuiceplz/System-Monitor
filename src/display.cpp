#include "../include/display.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

Display::Display() : mainWindow(nullptr), cpuWindow(nullptr), memoryWindow(nullptr), diskWindow(nullptr),
                     logWindow(nullptr), processWindow(nullptr), networkWindow(nullptr), 
                     processListScrollPosition(0), needsUpdate(false), networkWindowWidth(30) {
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
    endwin();
}

void Display::initializeScreen() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    if (has_colors() == FALSE) {
        endwin();
        throw std::runtime_error("Your terminal does not support color");
    }

    start_color();
    use_default_colors();

    if (COLORS >= 8) {
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    }

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    mainWindow = newwin(yMax - 24, xMax - networkWindowWidth, 0, 0);
    cpuWindow = newwin(6, (xMax - networkWindowWidth) / 3, 0, 0);
    memoryWindow = newwin(6, (xMax - networkWindowWidth) / 3, 0, (xMax - networkWindowWidth) / 3);
    diskWindow = newwin(6, (xMax - networkWindowWidth) / 3, 0, 2 * (xMax - networkWindowWidth) / 3);
    processWindow = newwin(12, xMax - networkWindowWidth, yMax - 24, 0);
    logWindow = newwin(12, xMax - networkWindowWidth, yMax - 12, 0);
    networkWindow = newwin(yMax, networkWindowWidth, 0, xMax - networkWindowWidth);

    keypad(mainWindow, TRUE);
    keypad(cpuWindow, TRUE);
    keypad(memoryWindow, TRUE);
    keypad(diskWindow, TRUE);
    keypad(logWindow, TRUE);
    keypad(processWindow, TRUE);
    keypad(networkWindow, TRUE);
}

void Display::update(const SystemMonitor& monitor) {
    updateMainWindow(monitor);
    updateCPUWindow(monitor);
    updateMemoryWindow(monitor);
    updateDiskWindow(monitor);
    updateProcessWindow(monitor.getProcesses());
    updateNetworkInfo(monitor.getNetworkInterfaces());
    updateLogWindow();
}

void Display::updateMainWindow(const SystemMonitor& monitor) {
    wclear(mainWindow);
    int yMax, xMax;
    getmaxyx(mainWindow, yMax, xMax);

    box(mainWindow, 0, 0);

    if (COLORS >= 8) {
        wattron(mainWindow, COLOR_PAIR(1) | A_BOLD);
    } else {
        wattron(mainWindow, A_BOLD);
    }
    mvwprintw(mainWindow, 1, (xMax - 20) / 2, "System Monitor");
    if (COLORS >= 8) {
        wattroff(mainWindow, COLOR_PAIR(1) | A_BOLD);
    } else {
        wattroff(mainWindow, A_BOLD);
    }

    if (monitor.isGPUMonitoringAvailable()) {
        updateGPUInfo(monitor.getGPUInfo());
    }

    if (COLORS >= 8) {
        wattron(mainWindow, COLOR_PAIR(2));
    } else {
        wattron(mainWindow, A_UNDERLINE);
    }
    mvwprintw(mainWindow, yMax - 2, 2, "Press 'q' to quit, UP/DOWN to scroll processes");
    if (COLORS >= 8) {
        wattroff(mainWindow, COLOR_PAIR(2));
    } else {
        wattroff(mainWindow, A_UNDERLINE);
    }

    wrefresh(mainWindow);
}

void Display::updateCPUWindow(const SystemMonitor& monitor) {
    wclear(cpuWindow);
    box(cpuWindow, 0, 0);

    if (COLORS >= 8) {
        wattron(cpuWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattron(cpuWindow, A_BOLD);
    }
    mvwprintw(cpuWindow, 0, 2, "CPU");
    if (COLORS >= 8) {
        wattroff(cpuWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattroff(cpuWindow, A_BOLD);
    }

    mvwprintw(cpuWindow, 1, 2, "Model: %s", monitor.getCpuModel().c_str());
    mvwprintw(cpuWindow, 2, 2, "Usage: %.2f%%", monitor.getCpuUsage());

    wrefresh(cpuWindow);
}

void Display::updateMemoryWindow(const SystemMonitor& monitor) {
    wclear(memoryWindow);
    box(memoryWindow, 0, 0);

    if (COLORS >= 8) {
        wattron(memoryWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattron(memoryWindow, A_BOLD);
    }
    mvwprintw(memoryWindow, 0, 2, "Memory");
    if (COLORS >= 8) {
        wattroff(memoryWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattroff(memoryWindow, A_BOLD);
    }

    double totalMemoryGB = monitor.getTotalMemory() / (1024.0 * 1024 * 1024);
    mvwprintw(memoryWindow, 1, 2, "Total: %.2f GB", totalMemoryGB);
    mvwprintw(memoryWindow, 2, 2, "Usage: %.2f%%", monitor.getMemoryUsage());

    wrefresh(memoryWindow);
}

void Display::updateDiskWindow(const SystemMonitor& monitor) {
    wclear(diskWindow);
    box(diskWindow, 0, 0);

    if (COLORS >= 8) {
        wattron(diskWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattron(diskWindow, A_BOLD);
    }
    mvwprintw(diskWindow, 0, 2, "Disk");
    if (COLORS >= 8) {
        wattroff(diskWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattroff(diskWindow, A_BOLD);
    }

    double totalDiskGB = monitor.getTotalDiskSpace() / (1024.0 * 1024 * 1024);
    mvwprintw(diskWindow, 1, 2, "Total: %.2f GB", totalDiskGB);
    mvwprintw(diskWindow, 2, 2, "Usage: %.2f%%", monitor.getDiskUsage());
    mvwprintw(diskWindow, 3, 2, "Device: %s", monitor.getDiskName().c_str());

    wrefresh(diskWindow);
}


void Display::updateGPUInfo(const std::vector<GPUInfo>& gpuInfos) {
    int startY = 7;
    for (const auto& gpu : gpuInfos) {
        mvwprintw(mainWindow, startY++, 2, "GPU %d: %s", gpu.index, gpu.name.c_str());
        mvwprintw(mainWindow, startY, 4, "Temp: ");
        if (gpu.temperature >= 0) {
            mvwprintw(mainWindow, startY, 10, "%.1f°C", gpu.temperature);
        } else {
            mvwprintw(mainWindow, startY, 10, "N/A");
        }
        mvwprintw(mainWindow, startY, 20, "Util: ");
        if (gpu.gpuUtilization >= 0) {
            mvwprintw(mainWindow, startY, 26, "%.1f%%", gpu.gpuUtilization);
        } else {
            mvwprintw(mainWindow, startY, 26, "N/A");
        }
        mvwprintw(mainWindow, startY, 36, "Mem: ");
        if (gpu.memoryUtilization >= 0) {
            mvwprintw(mainWindow, startY, 41, "%.1f%%", gpu.memoryUtilization);
        } else {
            mvwprintw(mainWindow, startY, 41, "N/A");
        }
        if (gpu.fanSpeedAvailable) {
            mvwprintw(mainWindow, startY, 51, "Fan: %.1f%%", gpu.fanSpeed);
        }
        startY += 2;
    }
}

void Display::updateLogWindow() {
    wclear(logWindow);
    box(logWindow, 0, 0);

    if (COLORS >= 8) {
        wattron(logWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattron(logWindow, A_BOLD);
    }
    mvwprintw(logWindow, 0, 2, "Log Messages");
    if (COLORS >= 8) {
        wattroff(logWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattroff(logWindow, A_BOLD);
    }

    size_t startIndex = logMessages.size() > MAX_LOG_MESSAGES ? logMessages.size() - MAX_LOG_MESSAGES : 0;
    for (size_t i = 0; i < MAX_LOG_MESSAGES && startIndex + i < logMessages.size(); ++i) {
        mvwprintw(logWindow, i + 1, 2, "%s", logMessages[startIndex + i].c_str());
    }

    wrefresh(logWindow);
}

void Display::updateProcessWindow(const std::vector<ProcessInfo>& processes) {
    wclear(processWindow);
    box(processWindow, 0, 0);

    if (COLORS >= 8) {
        wattron(processWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattron(processWindow, A_BOLD);
    }
    mvwprintw(processWindow, 0, 2, "Process List (Use UP/DOWN to scroll)");
    if (COLORS >= 8) {
        wattroff(processWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattroff(processWindow, A_BOLD);
    }

    int row = 1;
    for (size_t i = processListScrollPosition; i < processes.size() && row < PROCESS_WINDOW_HEIGHT; ++i) {
        const auto& process = processes[i];
        mvwprintw(processWindow, row, 1, "%-20s CPU: %5.1f%% Mem: %5.1f MB",
                  process.name.c_str(), process.cpuUsage, process.memoryUsage);
        row++;
    }

    wrefresh(processWindow);
}

void Display::updateNetworkInfo(const std::vector<NetworkInterface>& interfaces) {
    wclear(networkWindow);
    box(networkWindow, 0, 0);

    if (COLORS >= 8) {
        wattron(networkWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattron(networkWindow, A_BOLD);
    }
    mvwprintw(networkWindow, 0, 2, "Network Information");
    if (COLORS >= 8) {
        wattroff(networkWindow, COLOR_PAIR(3) | A_BOLD);
    } else {
        wattroff(networkWindow, A_BOLD);
    }

    int row = 1;
    double totalDownloadSpeed = 0;
    double totalUploadSpeed = 0;
    unsigned long long totalBytesReceived = 0;
    unsigned long long totalBytesSent = 0;

    for (const auto& interface : interfaces) {
        mvwprintw(networkWindow, row++, 1, "%s (%s)", interface.name.c_str(), interface.type.c_str());
        mvwprintw(networkWindow, row++, 1, "IP: %s", interface.ipAddress.c_str());
        mvwprintw(networkWindow, row++, 1, "Down: %.2f MB/s", interface.downloadSpeed / (1024 * 1024));
        mvwprintw(networkWindow, row++, 1, "Up: %.2f MB/s", interface.uploadSpeed / (1024 * 1024));
        mvwprintw(networkWindow, row++, 1, "Max Down: %.2f MB/s", interface.maxDownloadSpeed / (1024 * 1024));
        mvwprintw(networkWindow, row++, 1, "Max Up: %.2f MB/s", interface.maxUploadSpeed / (1024 * 1024));
        row++;

        totalDownloadSpeed += interface.downloadSpeed;
        totalUploadSpeed += interface.uploadSpeed;
        totalBytesReceived += interface.bytesReceived;
        totalBytesSent += interface.bytesSent;
    }

    mvwprintw(networkWindow, row++, 1, "Total Network Usage:");
    mvwprintw(networkWindow, row++, 1, "Down: %.2f MB/s", totalDownloadSpeed / (1024 * 1024));
    mvwprintw(networkWindow, row++, 1, "Up: %.2f MB/s", totalUploadSpeed / (1024 * 1024));
    mvwprintw(networkWindow, row++, 1, "Total Received: %.2f GB", totalBytesReceived / (1024.0 * 1024 * 1024));
    mvwprintw(networkWindow, row++, 1, "Total Sent: %.2f GB", totalBytesSent / (1024.0 * 1024 * 1024));

    wrefresh(networkWindow);
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
    logMessages.push_back(message);
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
