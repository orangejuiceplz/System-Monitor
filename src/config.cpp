#include "../include/config.h"
#include <fstream>
#include <sstream>

Config::Config() {
    setDefaultValues();
}

void Config::setDefaultValues() {
    settings["update_interval_ms"] = "2000";
    settings["cpu_threshold"] = "80.0";
    settings["memory_threshold"] = "80.0";
    settings["disk_threshold"] = "90.0";
    settings["gpu_temp_threshold"] = "80.0";
}

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            settings[key] = value;
        }
    }

    return true;
}

bool Config::save(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    for (const auto& [key, value] : settings) {
        file << key << "=" << value << "\n";
    }

    return true;
}

template<typename T>
T Config::getValue(const std::string& key, T defaultValue) const {
    auto it = settings.find(key);
    if (it == settings.end()) {
        return defaultValue;
    }
    
    T value;
    std::istringstream iss(it->second);
    iss >> value;
    return value;
}

int Config::getUpdateIntervalMs() const {
    return getValue<int>("update_interval_ms", 2000);
}

double Config::getCpuThreshold() const {
    return getValue<double>("cpu_threshold", 80.0);
}

double Config::getMemoryThreshold() const {
    return getValue<double>("memory_threshold", 80.0);
}

double Config::getDiskThreshold() const {
    return getValue<double>("disk_threshold", 90.0);
}

double Config::getGpuTempThreshold() const {
    return getValue<double>("gpu_temp_threshold", 80.0);
}

void Config::setUpdateIntervalMs(int interval) {
    settings["update_interval_ms"] = std::to_string(interval);
}

void Config::setCpuThreshold(double threshold) {
    settings["cpu_threshold"] = std::to_string(threshold);
}

void Config::setMemoryThreshold(double threshold) {
    settings["memory_threshold"] = std::to_string(threshold);
}

void Config::setDiskThreshold(double threshold) {
    settings["disk_threshold"] = std::to_string(threshold);
}

void Config::setGpuTempThreshold(double threshold) {
    settings["gpu_temp_threshold"] = std::to_string(threshold);
}
