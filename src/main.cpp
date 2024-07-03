#include "../include/system_monitor.h"
#include "../include/display.h"
#include <chrono>
#include <thread>

int main() {
    SystemMonitor monitor;
    Display display;

    int ch;
    bool need_update = true;

    while (true) {
        if (need_update) {
            monitor.update();
            display.update(monitor);
            need_update = false;
        }

        ch = getch();
        if (ch == 'q' || ch == 'Q') {
            break;
        } else if (ch != ERR) {
            // Some input was received, update on next iteration
            need_update = true;
        } else {
            // No input, sleep for a short time
            napms(100);  // Sleep for 100 milliseconds
        }

        // Update every second regardless of input
        static auto last_update = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_update).count() >= 1) {
            need_update = true;
            last_update = now;
        }
    }

    return 0;
}
