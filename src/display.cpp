// src/display.cpp
#include "../include/display.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

Display::Display() : mainWindow(nullptr), logWindow(nullptr) {
    initializeScreen();
}

Display::~Display() {
    if (mainWindow) {
        delwin(mainWindow);
    }
    if (logWindow) {
        delwin(logWindow);
    }
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

    mainWindow = newwin(yMax - 12, xMax, 0, 0);
    logWindow = newwin(12, xMax, yMax - 12, 0);

    keypad(mainWindow, TRUE);
    keypad(logWindow, TRUE);
}

void Display::update(const SystemMonitor& monitor) {
    updateMainWindow(monitor);
    updateGPUInfo(monitor.getGPUInfo());
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

    // Adjust the number of processes shown based on available space
    int availableLines = yMax - 15; // Reserve space for GPU info
    int processesToShow = std::min(5, availableLines);

    mvwprintw(mainWindow, 7, 2, "Top %d Processes by CPU Usage:", processesToShow);
    auto processes = monitor.getProcesses();
    std::sort(processes.begin(), processes.end(), 
              [](const ProcessInfo& a, const ProcessInfo& b) { return a.cpuUsage > b.cpuUsage; });

    for (int i = 0; i < processesToShow && i < static_cast<int>(processes.size()); ++i) {
        const auto& p = processes[i];
        mvwprintw(mainWindow, 9 + i, 2, "%-20s (PID: %5d): CPU %.2f%%, Mem %.2f MB", 
                 p.name.c_str(), p.pid, p.cpuUsage, p.memoryUsage);
    }

    if (COLORS >= 8) {
        wattron(mainWindow, COLOR_PAIR(2));
    } else {
        wattron(mainWindow, A_UNDERLINE);
    }
    mvwprintw(mainWindow, yMax - 2, 2, "Press 'q' to quit");
    if (COLORS >= 8) {
        wattroff(mainWindow, COLOR_PAIR(2));
    } else {
        wattroff(mainWindow, A_UNDERLINE);
    }

    wrefresh(mainWindow);
}

void Display::updateGPUInfo(const std::vector<GPUInfo>& gpuInfos) {
    int startY = 15; 
    for (const auto& gpu : gpuInfos) {
        mvwprintw(mainWindow, startY++, 2, "GPU %d: %s", gpu.index, gpu.name.c_str());
        mvwprintw(mainWindow, startY++, 4, "Temp: %.1f°C, Util: %.1f%%, Mem: %.1f%%", 
                  gpu.temperature, gpu.gpuUtilization, gpu.memoryUtilization);
        mvwprintw(mainWindow, startY++, 4, "Power: %.1fW, Fan: %.1f%%", 
                  gpu.powerUsage, gpu.fanSpeed);
        startY++; 
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

bool Display::handleInput() {
    int ch = wgetch(mainWindow);
    return (ch != 'q' && ch != 'Q');
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
