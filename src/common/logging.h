#ifndef CSCN74000_PROJECT_LOGGING_H
#define CSCN74000_PROJECT_LOGGING_H

#include <string>
#include <fstream>

#include "packets.h"

class Logger {
public:
    Logger();
    ~Logger();

    //Deleted copy constructor and assignment.
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Logs a system event with a timestamp.
     * @param event Description of the event.
     */
    void logEvent(const std::string& event);

    /**
     * @brief Logs a sent or received packet.
     * @param sender The ID or name of the sender.
     * @param direction "RX" or "TX".
     * @param packetType The packet type.
     * @param payloadSize The size of the packet in bytes.
     * @param checksum The packet checksum.
     * @param extra Optional extra metadata or content.
     */
    void logPacket(const std::string& sender,
                   const std::string& direction,
                   PacketType packetType,
                   uint32_t payloadSize,
                   uint32_t checksum,
                   const std::string& extra = "");

private:
    std::ofstream logFile;
    void openLogFile(); // Creates timestamped log file
};

#endif

