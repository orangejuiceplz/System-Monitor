#pragma once

#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    Logger(const std::string& filename);
    ~Logger();

    void logWarning(const std::string& message);
    void logError(const std::string& message);

private:
    std::ofstream logFile;
    std::mutex logMutex;

    std::string getTimestamp();
};
