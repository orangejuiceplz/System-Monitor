#include "../include/system_monitor.h"
#include "../include/display.h"
#include <chrono>
#include <thread>

int main() {
    SystemMonitor monitor;
    Display display;

    while (true) {
        monitor.update();
        display.update(monitor);

        if (!display.handleInput()) {
            break;  // Exit the loop if 'q' is pressed
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
