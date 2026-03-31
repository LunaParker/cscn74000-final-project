#include "logging.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

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
    // Log filename format: YYYY-MM-DD_HH-MM-SS_log.txt
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%d_%H-%M-%S") << "_UTC_log.txt";
    logFile.open(ss.str(), std::ios::out);
}

void Logger::logEvent(const std::string &sender, const std::string &direction, const std::string &message) {
    if (logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        // Required fields for logging: Timestamp, Direction, Sender
        logFile << "[" << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%d %H:%M:%S UTC") << "] "
                << "Dir: " << direction << " | "
                << "From: " << sender << " | "
                << "Data: " << message << '\n';
    }
}
