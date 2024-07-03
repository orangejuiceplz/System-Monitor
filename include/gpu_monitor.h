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

    void checkNVMLError(nvmlReturn_t result, const char* message);
};
