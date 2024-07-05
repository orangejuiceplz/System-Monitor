#include "../include/network_monitor.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <iostream>

NetworkMonitor::NetworkMonitor() {
    lastUpdateTime = std::chrono::steady_clock::now();
}

void NetworkMonitor::update() {
    auto currentTime = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration<double>(currentTime - lastUpdateTime).count();

    std::ifstream netdev("/proc/net/dev");
    std::string line;
    std::getline(netdev, line); // Skip header lines
    std::getline(netdev, line);

    while (std::getline(netdev, line)) {
        std::istringstream iss(line);
        std::string interfaceName;
        long long bytesReceived, bytesSent;

        iss >> interfaceName;
        interfaceName.pop_back(); // Remove colon

        for (int i = 0; i < 9; ++i) iss >> bytesReceived;
        iss >> bytesSent;

        auto it = std::find_if(interfaces.begin(), interfaces.end(),
                               [&](const NetworkInterface& i) { return i.name == interfaceName; });

        if (it == interfaces.end()) {
            interfaces.push_back({interfaceName, getInterfaceType(interfaceName), 
                                  bytesReceived, bytesSent, 0, 0, 0, 0});
            it = interfaces.end() - 1;
        }

        double downloadSpeed = (bytesReceived - it->bytesReceived) / elapsedSeconds;
        double uploadSpeed = (bytesSent - it->bytesSent) / elapsedSeconds;

        it->downloadSpeed = downloadSpeed;
        it->uploadSpeed = uploadSpeed;
        it->maxDownloadSpeed = std::max(it->maxDownloadSpeed, downloadSpeed);
        it->maxUploadSpeed = std::max(it->maxUploadSpeed, uploadSpeed);
        it->bytesReceived = bytesReceived;
        it->bytesSent = bytesSent;
    }

    lastUpdateTime = currentTime;
}

std::vector<NetworkInterface> NetworkMonitor::getNetworkInterfaces() const {
    return interfaces;
}

std::string NetworkMonitor::getInterfaceType(const std::string& name) {
    std::string path = "/sys/class/net/" + name;
    if (std::filesystem::exists(path + "/wireless")) {
        return "wireless";
    } else if (std::filesystem::exists(path + "/device")) {
        return "ethernet";
    } else if (name == "lo") {
        return "loopback";
    }
    return "unknown";
}
