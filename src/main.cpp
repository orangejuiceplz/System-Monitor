#include <dlfcn.h>
#include <iostream>
#include "../include/system_monitor.h"
#include "../include/display.h"
#include "../include/config.h"
#include "../include/logger.h"
#include <chrono>
#include <thread>
#include <memory>

int main() {
    void* nvml_handle = dlopen("libnvidia-ml.so", RTLD_LAZY);
    bool nvml_available = true;
    if (!nvml_handle) {
        std::cerr << "Warning: Failed to load NVML library: " << dlerror() << std::endl;
        std::cerr << "GPU monitoring will be disabled." << std::endl;
        nvml_available = false;
    } else {
        dlclose(nvml_handle);
    }

    Config config;
    if (!config.load("system_monitor.conf")) {
        std::cout << "Failed to load configuration. Using default values.\n";
    }

    auto logger = std::make_shared<Logger>("system_monitor.log");

    Display display;
    SystemMonitor monitor(config, logger, display, nvml_available);

    if (!monitor.initialize()) {
        std::cerr << "Failed to initialize system monitor" << std::endl;
        return 1;
    }

    logger->logWarning("System Monitor started");
    display.addLogMessage("System Monitor started");

    try {
        while (true) {
            monitor.update();
            display.update(monitor);
            
            if (!display.handleInput()) {
                logger->logWarning("System Monitor stopped");
                display.addLogMessage("System Monitor stopped");
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& e) {
        logger->logError("Unexpected error: " + std::string(e.what()));
        std::cerr << "An unexpected error occurred. Check the log file for details." << std::endl;
        return 1;
    }

    return 0;
}