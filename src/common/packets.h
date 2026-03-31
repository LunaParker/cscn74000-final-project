#ifndef CSCN74000_PROJECT_PACKETS_H
#define CSCN74000_PROJECT_PACKETS_H

#include <cstdint>

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
    FILE_DATA = 3U
};

/**
 * @brief Standard Packet Header
 */
struct PacketHeader {
    int32_t aircraftId;
    PacketType packetType;
    uint32_t payloadSize;
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
 * @brief Payload sent by the client during the VERIFY handshake.
 */
struct VerifyPayload {
    int32_t  aircraftId;
    uint32_t protocolVersion;
};

/**
 * @brief Response sent by the server during the VERIFY handshake.
 */
struct VerifyResponse {
    uint32_t accepted;
    uint32_t protocolVersion;
};
#endif
