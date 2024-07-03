#pragma once

#include "system_monitor.h"
#include <ncurses.h>

class Display {
public:
    Display();
    ~Display();

    void update(const SystemMonitor& monitor);
    bool handleInput();

private:
    void initializeScreen();
    WINDOW* window;  
};
