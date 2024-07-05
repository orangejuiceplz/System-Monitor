#pragma once

#include <vector>
#include <string>
#include <nvml.h>

struct GPUInfo {
    int index;
    std::string name;
    float temperature;
    float powerUsage;
    float fanSpeed;
    float gpuUtilization;
    float memoryUtilization;
    bool fanSpeedAvailable;
};

class GPUMonitor {
public:
    GPUMonitor();
    ~GPUMonitor();

    bool initialize();
    void update();
    std::vector<GPUInfo> getGPUInfo() const;

private:
    std::vector<GPUInfo> gpuInfos;
    nvmlReturn_t result;
    unsigned int deviceCount;
    std::vector<bool> fanUnavailabilityLogged;

    void checkNVMLError(nvmlReturn_t result, const char* message);
};
