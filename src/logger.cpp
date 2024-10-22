#include "../include/logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Unable to open log file: " + filename);
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::logWarning(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFile << getTimestamp() << " [WARNING] " << message << std::endl;
}

void Logger::logError(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFile << getTimestamp() << " [ERROR] " << message << std::endl;
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Logger::logInfo(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFile << getTimestamp() << " [INFO] " << message << std::endl;
}