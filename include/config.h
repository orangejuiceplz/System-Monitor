#pragma once

#include <string>
#include <unordered_map>

class Config {
public:
    Config();
    bool load(const std::string& filename);
    bool save(const std::string& filename) const;

    int getUpdateInterval() const;
    double getCpuThreshold() const;
    double getMemoryThreshold() const;
    double getDiskThreshold() const;

    void setUpdateInterval(int interval);
    void setCpuThreshold(double threshold);
    void setMemoryThreshold(double threshold);
    void setDiskThreshold(double threshold);

private:
    std::unordered_map<std::string, std::string> settings;
    
    void setDefaultValues();
    template<typename T>
    T getValue(const std::string& key, T defaultValue) const;
};
