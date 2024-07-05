#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>

struct NetworkInterface {
    std::string name;
    std::string type;
    unsigned long long bytesReceived;
    unsigned long long bytesSent;
    double downloadSpeed;
    double uploadSpeed;
    double maxDownloadSpeed;
    double maxUploadSpeed;
};

class NetworkMonitor {
public:
    NetworkMonitor();
    void update();
    std::vector<NetworkInterface> getActiveInterfaces() const;

private:
    std::unordered_map<std::string, NetworkInterface> interfaces;
    std::chrono::steady_clock::time_point lastUpdateTime;

    std::string getInterfaceType(const std::string& name) const;
    bool isInterfaceActive(const std::string& name) const;
    unsigned long long readSysfsValue(const std::string& interface, const std::string& file) const;
};
