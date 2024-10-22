#pragma once

#include <string>

class BatteryMonitor {
public:
    BatteryMonitor();
    void update();
    std::string getState() const;
    double getPercentage() const;
    std::string getEstimatedTime() const;

private:
    std::string state;
    double percentage;
    std::string estimatedTime;

    std::string readSysfsFile(const std::string& path) const;
    bool isBatteryPresent() const;
};