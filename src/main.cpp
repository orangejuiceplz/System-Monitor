#include "../include/system_monitor.h"
#include "../include/display.h"
#include "../include/config.h"
#include "../include/logger.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>

int main() {
    Config config;
    if (!config.load("system_monitor.conf")) {
        std::cout << "Failed to load configuration. Using default values.\n";
    }

    auto logger = std::make_shared<Logger>("system_monitor.log");
    logger->setLogLevel(LogLevel::INFO);

    Display display;
    SystemMonitor monitor(config, logger, display);

    logger->log(LogLevel::INFO, "System Monitor started");
    display.addLogMessage("System Monitor started");

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
            logger->log(LogLevel::INFO, "System Monitor stopped");
            display.addLogMessage("System Monitor stopped");
            break;
        } else if (ch != ERR) {
            need_update = true;
        } else {
            napms(100);
        }

        static auto last_update = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_update).count() >= config.getUpdateInterval()) {
            need_update = true;
            last_update = now;
        }

        if (monitor.isAlertTriggered()) {
            display.showAlert("System threshold exceeded!");
        }   
    }

    return 0;
}
