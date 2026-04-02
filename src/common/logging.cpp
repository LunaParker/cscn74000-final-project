#include "logging.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

Logger::Logger() {
    openLogFile();
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::openLogFile() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    // Log filename format: YYYY-MM-DD_HH-MM-SS_log.txt
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%d_%H-%M-%S") << "_UTC_log.txt";
    logFile.open(ss.str(), std::ios::out);
}

void Logger::logEvent(const std::string& event) {
    if (logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        logFile << "["
                << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%d %H:%M:%S UTC")
                << "] "
                << event
                << '\n';
    }
}

void Logger::logPacket(const std::string& sender,
                       const std::string& direction,
                       PacketType packetType,
                       uint32_t payloadSize,
                       uint32_t checksum,
                       const std::string& extra) {
    std::stringstream ss;
    ss << "Sender: " << sender
       << " | Direction: " << direction
       << " | Packet Type: " << packetTypeToString(packetType)
       << " | Payload Size: " << payloadSize
       << " | Checksum: " << checksum;

    if (!extra.empty()) {
        ss << " | " << extra;
    }

    logEvent(ss.str());
}

