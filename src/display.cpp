#include "../include/display.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

Display::Display() {
    initializeScreen();
}

Display::~Display() {
    endwin();
}

void Display::initializeScreen() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
    }
}

void Display::update(const SystemMonitor& monitor) {
    clear();
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    // Draw border
    box(stdscr, 0, 0);

    // Title
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, (xMax - 20) / 2, "System Monitor");
    attroff(COLOR_PAIR(1) | A_BOLD);

    // System info
    mvprintw(3, 2, "CPU Usage:    %.2f%%", monitor.getCpuUsage());
    mvprintw(4, 2, "Memory Usage: %.2f%%", monitor.getMemoryUsage());
    mvprintw(5, 2, "Disk Usage:   %.2f%%", monitor.getDiskUsage());

    // Process list
    mvprintw(7, 2, "Top 5 Processes by CPU Usage:");
    auto processes = monitor.getProcesses();
    std::sort(processes.begin(), processes.end(), 
              [](const ProcessInfo& a, const ProcessInfo& b) { return a.cpuUsage > b.cpuUsage; });

    for (int i = 0; i < std::min(5, static_cast<int>(processes.size())); ++i) {
        const auto& p = processes[i];
        mvprintw(9 + i, 2, "%-20s (PID: %5d): CPU %.2f%%, Mem %.2f MB", 
                 p.name.c_str(), p.pid, p.cpuUsage, p.memoryUsage);
    }

    // Instructions
    attron(COLOR_PAIR(2));
    mvprintw(yMax - 2, 2, "Press 'q' to quit");
    attroff(COLOR_PAIR(2));

    refresh();
}

bool Display::handleInput() {
    int ch = getch();
    return (ch != 'q' && ch != 'Q');
}
