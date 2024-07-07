#include "../include/battery_monitor.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <filesystem>

BatteryMonitor::BatteryMonitor() : state("Unknown"), percentage(0), estimatedTime("N/A") {
    update();
}

void BatteryMonitor::update() {
    if (!isBatteryPresent()) {
        state = "No Battery";
        percentage = 0;
        estimatedTime = "N/A";
        return;
    }

    state = readSysfsFile("/sys/class/power_supply/BAT0/status");
    
    std::string energyNow = readSysfsFile("/sys/class/power_supply/BAT0/energy_now");
    std::string energyFull = readSysfsFile("/sys/class/power_supply/BAT0/energy_full");
    
    if (!energyNow.empty() && !energyFull.empty()) {
        double now = std::stod(energyNow);
        double full = std::stod(energyFull);
        percentage = (now / full) * 100.0;
    }

    std::string powerNow = readSysfsFile("/sys/class/power_supply/BAT0/power_now");
    if (!powerNow.empty() && state == "Discharging") {
        double power = std::stod(powerNow);
        if (power > 0) {
            double timeLeft = (std::stod(energyNow) / power);
            int hours = static_cast<int>(timeLeft);
            int minutes = static_cast<int>((timeLeft - hours) * 60);
            estimatedTime = std::to_string(hours) + "h " + std::to_string(minutes) + "m";
        } else {
            estimatedTime = "N/A";
        }
    } else {
        estimatedTime = "N/A";
    }
}

std::string BatteryMonitor::getState() const {
    return state;
}

double BatteryMonitor::getPercentage() const {
    return percentage;
}

std::string BatteryMonitor::getEstimatedTime() const {
    return estimatedTime;
}

std::string BatteryMonitor::readSysfsFile(const std::string& path) const {
    std::ifstream file(path);
    std::string content;
    if (file.is_open()) {
        std::getline(file, content);
    }
    return content;
}

bool BatteryMonitor::isBatteryPresent() const {
    return std::filesystem::exists("/sys/class/power_supply/BAT0");
}
