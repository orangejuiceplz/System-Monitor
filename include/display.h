#pragma once

#include "system_monitor.h"
#include <ncurses.h>

class Display {
public:
    Display();
    ~Display();

    void update(const SystemMonitor& monitor);
    bool handleInput();
    void showAlert(const std::string& message);
private:
    void initializeScreen();
    WINDOW* window;  
    WINDOW* alertWindow;
};
