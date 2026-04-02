#ifndef CSCN74000_PROJECT_PACKETS_H
#define CSCN74000_PROJECT_PACKETS_H

#include <cstdint>
#include <string>

/**
 * @brief Platform-agnostic socket type aliases.
 *
 * Windows uses the SOCKET handle type from Winsock2.
 * POSIX systems use a plain int file descriptor.
 */
#ifdef _WIN32
#include <winsock2.h>
using SocketHandle = SOCKET;
const SocketHandle INVALID_SOCK = INVALID_SOCKET;
#else
using SocketHandle = int;
constexpr SocketHandle INVALID_SOCK = -1;
#endif

/// Maximum payload size in bytes for any single packet.
constexpr uint32_t MAX_PAYLOAD_SIZE = 1024U;

/**
 * @brief Enumeration of supported packet types
 */

enum class PacketType : uint32_t {
    VERIFY = 0U,
    TELEMETRY = 1U,
    FILE_METADATA = 2U,
    FILE_DATA = 3U,
    ACK = 4U,
    ERROR = 5U,
};

/**
 * @brief Standard Packet Header
 */
struct PacketHeader {
    int32_t aircraftId;
    PacketType packetType;
    uint32_t payloadSize;
    uint32_t checksum;
};

/**
 * @brief Enumeration of server connection states for the state machine.
 */
enum class ConnectionState : uint32_t {
    LISTENING    = 0U,
    CONNECTED    = 1U,
    VERIFYING    = 2U,
    RECEIVING    = 3U,
    DISCONNECTED = 4U
};

/// Protocol version for VERIFY handshake compatibility checks.
constexpr uint32_t ATS_PROTOCOL_VERSION = 1U;

/**
 * @brief Telemetry payload containing timestamped spatial coordinates.
 */
struct TelemetryPayload {
    int64_t timestamp;
    double latitude;
    double longitude;
    double altitude;
};

/**
 * @brief Converts a PacketType enum value to a human-readable string.
 *
 * This helper function is primarily used for logging so that packet types can
 * be displayed in a readable format instead of as raw numeric values.
 *
 * @param type The PacketType value to convert.
 * @return A string representation of the packet type (e.g. "VERIFY",
 *         "TELEMETRY", "FILE_DATA", "ACK"). Returns "UNKNOWN" if the packet
 *         type does not match a recognized enum value.
 */
inline static std::string packetTypeToString(PacketType type) {
    switch (type) {
        case PacketType::VERIFY:            return "VERIFY";
        case PacketType::TELEMETRY:         return "TELEMETRY";
        case PacketType::FILE_METADATA:     return "FILE_METADATA";
        case PacketType::FILE_DATA:         return "FILE_DATA";
        case PacketType::ACK:               return "ACK";
        case PacketType::ERROR:             return "ERROR";
        default:                            return "UNKNOWN";
    }
}

#endif

