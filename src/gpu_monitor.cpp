#include "../include/gpu_monitor.h"
#include <stdexcept>
#include <iostream>
#include <thread>
#include <chrono>

GPUMonitor::GPUMonitor() : result(NVML_SUCCESS), deviceCount(0) {}

GPUMonitor::~GPUMonitor() {
    if (result == NVML_SUCCESS) {
        nvmlShutdown();
    }
}

bool GPUMonitor::initialize() {
    result = nvmlInit();
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to initialize NVML: " << nvmlErrorString(result) << std::endl;
        return false;
    }

    result = nvmlDeviceGetCount(&deviceCount);
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to get device count: " << nvmlErrorString(result) << std::endl;
        return false;
    }

    gpuInfos.resize(deviceCount);
    return true;
}

void GPUMonitor::update() {
    for (unsigned int i = 0; i < deviceCount; i++) {
        nvmlDevice_t device;
        result = nvmlDeviceGetHandleByIndex(i, &device);
        checkNVMLError(result, "Failed to get device handle");

        gpuInfos[i].index = i;

        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        result = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
        checkNVMLError(result, "Failed to get device name");
        gpuInfos[i].name = name;

        unsigned int temperature;
        result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature);
        if (result == NVML_SUCCESS) {
            gpuInfos[i].temperature = static_cast<float>(temperature);
        } else {
            std::cerr << "Failed to get GPU temperature: " << nvmlErrorString(result) << std::endl;
            gpuInfos[i].temperature = -1.0f;
        }

        unsigned int power;
        result = nvmlDeviceGetPowerUsage(device, &power);
        if (result == NVML_SUCCESS) {
            gpuInfos[i].powerUsage = static_cast<float>(power) / 1000.0f;
        } else {
            std::cerr << "Failed to get GPU power usage: " << nvmlErrorString(result) << std::endl;
            gpuInfos[i].powerUsage = -1.0f;
        }

        unsigned int fanSpeed;
        result = nvmlDeviceGetFanSpeed(device, &fanSpeed);
        if (result == NVML_SUCCESS) {
            gpuInfos[i].fanSpeed = static_cast<float>(fanSpeed);
        } else {
            std::cerr << "Failed to get GPU fan speed: " << nvmlErrorString(result) << std::endl;
            gpuInfos[i].fanSpeed = -1.0f;
        }

        nvmlUtilization_t utilization;
        result = nvmlDeviceGetUtilizationRates(device, &utilization);
        if (result == NVML_SUCCESS) {
            gpuInfos[i].gpuUtilization = static_cast<float>(utilization.gpu);
            gpuInfos[i].memoryUtilization = static_cast<float>(utilization.memory);
        } else {
            std::cerr << "Failed to get GPU utilization: " << nvmlErrorString(result) << std::endl;
            gpuInfos[i].gpuUtilization = -1.0f;
            gpuInfos[i].memoryUtilization = -1.0f;
        }

        nvmlMemory_t memInfo;
        result = nvmlDeviceGetMemoryInfo(device, &memInfo);
        if (result == NVML_SUCCESS) {
            gpuInfos[i].memoryUtilization = static_cast<float>(memInfo.used) / static_cast<float>(memInfo.total) * 100.0f;
        } else {
            std::cerr << "Failed to get GPU memory info: " << nvmlErrorString(result) << std::endl;
            gpuInfos[i].memoryUtilization = -1.0f;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

std::vector<GPUInfo> GPUMonitor::getGPUInfo() const {
    return gpuInfos;
}

void GPUMonitor::checkNVMLError(nvmlReturn_t result, const char* message) {
    if (result != NVML_SUCCESS) {
        throw std::runtime_error(std::string(message) + ": " + nvmlErrorString(result));
    }
}
