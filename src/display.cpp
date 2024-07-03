#include "../include/display.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

Display::Display() : window(nullptr) {
    initializeScreen();
}

Display::~Display() {
    if (window) {
        delwin(window);
    }
    endwin();
}

void Display::initializeScreen() {
    window = initscr();
    if (!window) {
        throw std::runtime_error("Failed to initialize ncurses");
    }

    cbreak();
    noecho();
    keypad(window, TRUE);
    nodelay(window, TRUE);
    curs_set(0);

    if (has_colors() == FALSE) {
        endwin();
        throw std::runtime_error("Your terminal does not support color");
    }

    start_color();
    use_default_colors();

    mvwprintw(window, 0, 0, "Terminal has %d colors", COLORS);
    mvwprintw(window, 1, 0, "Terminal has %d color pairs", COLOR_PAIRS);
    wrefresh(window);
    napms(2000);  

    if (COLORS >= 8) {
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
    }
}

void Display::update(const SystemMonitor& monitor) {
    wclear(window);
    int yMax, xMax;
    getmaxyx(window, yMax, xMax);

    box(window, 0, 0);

    if (COLORS >= 8) {
        wattron(window, COLOR_PAIR(1) | A_BOLD);
    } else {
        wattron(window, A_BOLD);
    }
    mvwprintw(window, 1, (xMax - 20) / 2, "System Monitor");
    if (COLORS >= 8) {
        wattroff(window, COLOR_PAIR(1) | A_BOLD);
    } else {
        wattroff(window, A_BOLD);
    }

    mvwprintw(window, 3, 2, "CPU Usage:    %.2f%%", monitor.getCpuUsage());
    mvwprintw(window, 4, 2, "Memory Usage: %.2f%%", monitor.getMemoryUsage());
    mvwprintw(window, 5, 2, "Disk Usage:   %.2f%%", monitor.getDiskUsage());

    mvwprintw(window, 7, 2, "Top 5 Processes by CPU Usage:");
    auto processes = monitor.getProcesses();
    std::sort(processes.begin(), processes.end(), 
              [](const ProcessInfo& a, const ProcessInfo& b) { return a.cpuUsage > b.cpuUsage; });

    for (int i = 0; i < std::min(5, static_cast<int>(processes.size())); ++i) {
        const auto& p = processes[i];
        mvwprintw(window, 9 + i, 2, "%-20s (PID: %5d): CPU %.2f%%, Mem %.2f MB", 
                 p.name.c_str(), p.pid, p.cpuUsage, p.memoryUsage);
    }

    if (COLORS >= 8) {
        wattron(window, COLOR_PAIR(2));
    } else {
        wattron(window, A_UNDERLINE);
    }
    mvwprintw(window, yMax - 2, 2, "Press 'q' to quit");
    if (COLORS >= 8) {
        wattroff(window, COLOR_PAIR(2));
    } else {
        wattroff(window, A_UNDERLINE);
    }

    wrefresh(window);
}

bool Display::handleInput() {
    int ch = wgetch(window);
    return (ch != 'q' && ch != 'Q');
}

void Display::showAlert(const std::string& message) {
    int yMax, xMax;
    getmaxyx(window, yMax, xMax);

    int height = 3;
    int width = message.length() + 4;
    int startY = (yMax - height) / 2;
    int startX = (xMax - width) / 2;

    alertWindow = newwin(height, width, startY, startX);
    box(alertWindow, 0, 0);

    mvwprintw(alertWindow, 1, 2, message.c_str());
    wrefresh(alertWindow);

    wgetch(alertWindow);

    wclear(alertWindow);
    wrefresh(alertWindow);
    delwin(alertWindow);
    alertWindow = nullptr;

    touchwin(window);
    wrefresh(window);
}
