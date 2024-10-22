#include "../include/network_monitor.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

NetworkMonitor::NetworkMonitor() {
    lastUpdateTime = std::chrono::steady_clock::now();
}

void NetworkMonitor::update() {
    auto currentTime = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration<double>(currentTime - lastUpdateTime).count();

    for (const auto& entry : std::filesystem::directory_iterator("/sys/class/net/")) {
        std::string interfaceName = entry.path().filename();
        if (isInterfaceActive(interfaceName)) {
            std::string interfaceType = getInterfaceType(interfaceName);

            if (interfaceType == "ethernet" || interfaceType == "wireless") {
                unsigned long long bytesReceived = readSysfsValue(interfaceName, "statistics/rx_bytes");
                unsigned long long bytesSent = readSysfsValue(interfaceName, "statistics/tx_bytes");

                auto it = interfaces.find(interfaceName);
                if (it == interfaces.end()) {
                    interfaces[interfaceName] = {interfaceName, interfaceType, getIpAddress(interfaceName), bytesReceived, bytesSent, 0, 0, 0, 0};
                    it = interfaces.find(interfaceName);
                }

                double downloadSpeed = (bytesReceived - it->second.bytesReceived) / elapsedSeconds;
                double uploadSpeed = (bytesSent - it->second.bytesSent) / elapsedSeconds;

                it->second.downloadSpeed = downloadSpeed;
                it->second.uploadSpeed = uploadSpeed;
                it->second.maxDownloadSpeed = std::max(it->second.maxDownloadSpeed, downloadSpeed);
                it->second.maxUploadSpeed = std::max(it->second.maxUploadSpeed, uploadSpeed);
                it->second.bytesReceived = bytesReceived;
                it->second.bytesSent = bytesSent;
            }
        }
    }

    lastUpdateTime = currentTime;
}

std::vector<NetworkInterface> NetworkMonitor::getActiveInterfaces() const {
    std::vector<NetworkInterface> activeInterfaces;
    for (const auto& pair : interfaces) {
        if (isInterfaceActive(pair.first)) {
            activeInterfaces.push_back(pair.second);
        }
    }
    return activeInterfaces;
}

std::string NetworkMonitor::getInterfaceType(const std::string& name) const {
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

bool NetworkMonitor::isInterfaceActive(const std::string& name) const {
    std::string operstatePath = "/sys/class/net/" + name + "/operstate";
    std::ifstream operstate(operstatePath);
    std::string state;
    if (operstate >> state) {
        return state == "up";
    }
    return false;
}

unsigned long long NetworkMonitor::readSysfsValue(const std::string& interface, const std::string& file) const {
    std::string path = "/sys/class/net/" + interface + "/" + file;
    std::ifstream ifs(path);
    if (!ifs) {
        std::cerr << "Failed to open " << path << std::endl;
        return 0;
    }
    unsigned long long value;
    ifs >> value;
    return value;
}

std::string NetworkMonitor::getIpAddress(const std::string& interface) const {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return "";
    }

    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) == -1) {
        close(fd);
        return "";
    }

    close(fd);
    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}