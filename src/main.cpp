#include <iostream>
#include "../include/system_monitor.h"
#include "../include/display.h"
#include "../include/config.h"
#include "../include/logger.h"
#include <memory>

int main() {
    Config config;
    if (!config.load("system_monitor.conf")) {
        std::cout << "Failed to load configuration. Using default values.\n";
    }

    auto logger = std::make_shared<Logger>("system_monitor.log");

    Display display;
    SystemMonitor monitor(config, logger, display, true); 

    if (!monitor.initialize()) {
        std::cerr << "Failed to initialize system monitor" << std::endl;
        return 1;
    }

    logger->logInfo("System Monitor started");
    display.addLogMessage("System Monitor started.");
    display.addLogMessage("Use 'q' to quit the app and UP/DOWN arrows to scroll");


    try {
        monitor.run();
    } catch (const std::exception& e) {
        logger->logError("Unexpected error: " + std::string(e.what()));
        std::cerr << "An unexpected error occurred. Check the log file for details." << std::endl;
        return 1;
    }

    logger->logInfo("System Monitor stopped");
    display.addLogMessage("System Monitor stopped");

    return 0;
}
