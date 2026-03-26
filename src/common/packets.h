#ifndef CSCN74000_PROJECT_PACKETS_H
#define CSCN74000_PROJECT_PACKETS_H

#include <cstdint>

/**
 * @brief Standard Packet Header
 */
struct PacketHeader
{
    int32_t aircraftId;
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


