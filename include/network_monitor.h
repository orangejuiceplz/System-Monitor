#pragma once

#include <string>
#include <vector>
#include <chrono>

struct NetworkInterface {
    std::string name;
    std::string type;
    long long bytesReceived;
    long long bytesSent;
    double downloadSpeed;
    double uploadSpeed;
    double maxDownloadSpeed;
    double maxUploadSpeed;
};

class NetworkMonitor {
public:
    NetworkMonitor();
    void update();
    std::vector<NetworkInterface> getNetworkInterfaces() const;

private:
    std::vector<NetworkInterface> interfaces;
    std::chrono::steady_clock::time_point lastUpdateTime;
    void updateInterfaceStats(NetworkInterface& interface);
    std::string getInterfaceType(const std::string& name);
};
