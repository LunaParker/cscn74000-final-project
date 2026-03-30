#include "logging.h"
#include <chrono>
#include <iomanip>

Logger::Logger() {
    openDailyFile();
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::openDailyFile() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    // Format: YYYY-MM-DD_log.txt for traceability
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d") << "_log.txt";

    logFile.open(ss.str(), std::ios::app);
}

void Logger::logEvent(const std::string& sender, const std::string& direction, const std::string& message) {
    if (logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        // Required fields for logging: Timestamp, Direction, Sender
        logFile << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << "] "
                << "Dir: " << direction << " | "
                << "From: " << sender << " | "
                << "Data: " << message << '\n';
    }
}