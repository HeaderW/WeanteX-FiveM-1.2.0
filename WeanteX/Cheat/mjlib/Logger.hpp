#include <windows.h>
#include <iostream>
#include <string>
#include <map>
#include <ctime>

class Logger {
public:
    enum class LogLevel {
        LOG_NORMAL,
        LOG_SUCCESSFUL,
        LOG_ERROR,
        LOG_WARNING,
        LOG_INFO,
        LOG_DEBUG
    };

    static void WriteLog(const std::string& message, LogLevel level = Logger::LogLevel::LOG_NORMAL) {
        std::time_t time = std::time(nullptr);
        std::tm ptm;
        if (localtime_s(&ptm, &time) == 0) {
            char buffer[32];
            if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ptm)) {
                std::string timestamp(buffer);
                OutputToConsole(message, level, timestamp);
            }
        }
    }

private:
    static inline std::map<LogLevel, WORD> colorMap = {
        {Logger::LogLevel::LOG_NORMAL, 7},
        {Logger::LogLevel::LOG_SUCCESSFUL, 10},
        {Logger::LogLevel::LOG_ERROR, 4},
        {Logger::LogLevel::LOG_WARNING, 14},
        {Logger::LogLevel::LOG_INFO, 13},
        {Logger::LogLevel::LOG_DEBUG, 9},    
    };

    static void OutputToConsole(const std::string& message, LogLevel level, const std::string& timestamp) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (colorMap.find(level) != colorMap.end()) {
            SetConsoleTextAttribute(hConsole, colorMap[level]);
        }
        std::cout << "[" << timestamp << "] " << GetPrefix(level) << message << std::endl;
        SetConsoleTextAttribute(hConsole, 7);
    }

    static std::string GetPrefix(LogLevel level) {
        switch (level) {
        case Logger::LogLevel::LOG_SUCCESSFUL: return "[SUCCESSFUL]: ";
        case Logger::LogLevel::LOG_ERROR: return "[ERROR]: ";
        case Logger::LogLevel::LOG_WARNING: return "[WARNING]: ";
        case Logger::LogLevel::LOG_INFO: return "[INFO]: ";
        case Logger::LogLevel::LOG_DEBUG: return "[DEBUG]: ";
        default: return "[LOG]: ";
        }
    }
};
