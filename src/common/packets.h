#ifndef CSCN74000_PROJECT_PACKETS_H
#define CSCN74000_PROJECT_PACKETS_H

#include <cstdint>

/**
 * @brief Enumeration of supported packet types
 */

enum class PacketType : uint32_t {
    VERIFY = 0U,
    TELEMETRY = 1U,
    FILE_METADATA = 2U,
    FILE_DATA = 3U
};

/**
 * @brief Standard Packet Header
 */
struct PacketHeader
{
    int32_t aircraftId;
    PacketType packetType;
    uint32_t payloadSize;
};

/**
 * @brief Telemetry payload containing timestamped spatial coordinates.
 */
struct TelemetryPayload
{
    int64_t timestamp;
    double latitude;
    double longitude;
    double altitude;

};
#endif


