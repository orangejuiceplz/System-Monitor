#include "../include/display.h"
#include "../include/network_monitor.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

Display::Display() : mainWindow(nullptr), logWindow(nullptr), processWindow(nullptr), networkWindow(nullptr), processListScrollPosition(0), needsUpdate(false) {
    initializeScreen();
}

Display::~Display() {
    if (mainWindow) delwin(mainWindow);
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

    mainWindow = newwin(yMax - 36, xMax, 0, 0);
    processWindow = newwin(12, xMax, yMax - 36, 0);
    networkWindow = newwin(12, xMax, yMax - 24, 0);
    logWindow = newwin(12, xMax, yMax - 12, 0);

    keypad(mainWindow, TRUE);
    keypad(logWindow, TRUE);
    keypad(processWindow, TRUE);
    keypad(networkWindow, TRUE);
}

void Display::update(const SystemMonitor& monitor) {
    updateMainWindow(monitor);
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

    mvwprintw(mainWindow, 3, 2, "CPU Usage:    %.2f%%", monitor.getCpuUsage());
    mvwprintw(mainWindow, 4, 2, "Memory Usage: %.2f%%", monitor.getMemoryUsage());
    mvwprintw(mainWindow, 5, 2, "Disk Usage:   %.2f%%", monitor.getDiskUsage());

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
    for (const auto& interface : interfaces) {
        mvwprintw(networkWindow, row++, 1, "Interface: %s (%s)", interface.name.c_str(), interface.type.c_str());
        mvwprintw(networkWindow, row++, 1, "Download: %.2f MB/s (Max: %.2f MB/s)", 
                  interface.downloadSpeed / (1024 * 1024), interface.maxDownloadSpeed / (1024 * 1024));
        mvwprintw(networkWindow, row++, 1, "Upload: %.2f MB/s (Max: %.2f MB/s)", 
                  interface.uploadSpeed / (1024 * 1024), interface.maxUploadSpeed / (1024 * 1024));
        row++;
    }

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

