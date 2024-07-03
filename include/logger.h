#pragma once

#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    Logger(const std::string& filename);
    ~Logger();

    void log(LogLevel level, const std::string& message);
    void setLogLevel(LogLevel level);

private:
    std::ofstream logFile;
    LogLevel currentLevel;
    std::mutex logMutex;

    std::string getTimestamp();
    std::string logLevelToString(LogLevel level);
};
