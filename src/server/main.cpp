#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "common/logging.h"
#include "common/packets.h"
#include "main.h"

#include <array>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace CSCN_74000_SERVER {

// ---------------------------------------------------------------------------
// Signal handling — uses a function-local static to avoid a global variable.
// ---------------------------------------------------------------------------

/// Provides access to the shutdown flag without a namespace-scope variable.
static volatile std::sig_atomic_t &shutdownFlag() {
    static volatile std::sig_atomic_t flag = 0;
    return flag;
}

void signalHandler(int signum) {
    (void) signum;
    shutdownFlag() = 1;
}

bool isRunning() {
    return shutdownFlag() == 0;
}

// ---------------------------------------------------------------------------
// Platform socket helpers
// ---------------------------------------------------------------------------

bool initPlatformSockets() {
#ifdef _WIN32
    WSADATA wsaData;
    bool success = (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
    return success;
#else
    return true;
#endif
}

void cleanupPlatformSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void closeSocket(SocketHandle sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    // V2547: return value of close intentionally discarded — no recovery action on failure
    static_cast<void>(close(sock));
#endif
}

// ---------------------------------------------------------------------------
// Port resolution
// ---------------------------------------------------------------------------

uint16_t getPort() {
    uint16_t port = DEFAULT_PORT;

    // V2509: std::getenv is banned by MISRA but required to read runtime configuration
    constexpr char envVarName[] = "CSCN_74000_PORT"; //-V::2575
    const char *envPort = std::getenv(envVarName); //-V::2509
    if (envPort != nullptr) {
        char *end = nullptr;
        const unsigned long parsed = std::strtoul(envPort, &end, 10);

        // Accept only if the entire string was consumed and the value fits
        // in the valid port range (1–65535).
        const bool valid = (end != envPort) && (*end == '\0') &&
                           (parsed > 0U) && (parsed <= 65535U);
        if (valid) {
            port = static_cast<uint16_t>(parsed);
        }
    }

    return port;
}

// ---------------------------------------------------------------------------
// Socket creation
// ---------------------------------------------------------------------------

/// Sets SO_REUSEADDR, binds to the given port, and begins listening.
/// Returns true on success, false if any step fails.
static bool configureListenSocket(SocketHandle sock, uint16_t port) {
    int opt = 1;
    bool success = false;

    // setsockopt signature differs between platforms:
    //   Windows: const char*   for optval
    //   POSIX:   const void*   for optval
#ifdef _WIN32
    const int optResult = setsockopt(
        sock, SOL_SOCKET, SO_REUSEADDR,
        reinterpret_cast<const char *>(&opt), sizeof(opt));
#else
    const int optResult = setsockopt(
        sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    if (optResult == 0) {
        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) — required by bind() API
        const int bindResult = bind(
            sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));

        if (bindResult == 0) {
            success = (listen(sock, SOMAXCONN) == 0);
        }
    }

    return success;
}

SocketHandle createListenSocket(uint16_t port) {
    SocketHandle result = INVALID_SOCK;
    SocketHandle sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock != INVALID_SOCK) {
        if (configureListenSocket(sock, port)) {
            result = sock;
        } else {
            closeSocket(sock);
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// Reliable receive
// ---------------------------------------------------------------------------

bool recvAll(SocketHandle sock, char *buffer, uint32_t length) {
    uint32_t totalReceived = 0U;
    bool success = true;

    while ((totalReceived < length) && success) {
        const auto remaining = static_cast<int>(length - totalReceived);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) — offset into caller's buffer
        const auto bytesRead = recv(sock, &buffer[totalReceived], remaining, 0); //-V::2563

        if (bytesRead <= 0) {
            success = false;
        } else {
            totalReceived += static_cast<uint32_t>(bytesRead);
        }
    }

    return success;
}

// ---------------------------------------------------------------------------
// Reliable send
// ---------------------------------------------------------------------------

bool sendAll(SocketHandle sock, const char *buffer, uint32_t length) {
    uint32_t totalSent = 0U;
    bool success = true;

    while ((totalSent < length) && success) {
        const auto remaining = static_cast<int>(length - totalSent);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) — offset into caller's buffer
        const auto bytesSent = send(sock, &buffer[totalSent], remaining, 0); //-V::2563

        if (bytesSent <= 0) {
            success = false;
        } else {
            totalSent += static_cast<uint32_t>(bytesSent);
        }
    }

    return success;
}

// ---------------------------------------------------------------------------
// Packet helpers
// ---------------------------------------------------------------------------

/// Converts a PacketType enum to a human-readable string for logging.
const char *packetTypeName(PacketType type) {
    const char *name = "UNKNOWN";

    switch (type) {
        case PacketType::VERIFY:
            name = "VERIFY";
            break;
        case PacketType::TELEMETRY:
            name = "TELEMETRY";
            break;
        case PacketType::FILE_METADATA:
            name = "FILE_METADATA";
            break;
        case PacketType::FILE_DATA:
            name = "FILE_DATA";
            break;
        default:
            // Intentionally empty — all valid PacketType values handled above
            break;
    }

    return name;
}

/// Converts a ConnectionState enum to a human-readable string for logging.
const char *connectionStateName(ConnectionState state) {
    const char *name = "UNKNOWN";

    switch (state) {
        case ConnectionState::LISTENING:
            name = "LISTENING";
            break;
        case ConnectionState::CONNECTED:
            name = "CONNECTED";
            break;
        case ConnectionState::VERIFYING:
            name = "VERIFYING";
            break;
        case ConnectionState::RECEIVING:
            name = "RECEIVING";
            break;
        case ConnectionState::DISCONNECTED:
            name = "DISCONNECTED";
            break;
        default:
            // Intentionally empty — all valid ConnectionState values handled above
            break;
    }

    return name;
}

// ---------------------------------------------------------------------------
// Client handling
// ---------------------------------------------------------------------------

/// Constructs and sends a VerifyResponse packet to the client.
/// Returns true if both the header and payload were sent successfully.
static bool sendVerifyResponse(SocketHandle client, uint32_t accepted) {
    PacketHeader responseHeader{};
    responseHeader.aircraftId = 0;
    responseHeader.packetType = PacketType::VERIFY;
    responseHeader.payloadSize = sizeof(VerifyResponse);

    VerifyResponse response{};
    response.accepted = accepted;
    response.protocolVersion = ATS_PROTOCOL_VERSION;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) — required to send raw struct bytes
    const bool hdrSent = sendAll(
        client, reinterpret_cast<const char *>(&responseHeader),
        sizeof(responseHeader));

    bool bodySent = false;
    if (hdrSent) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) — required to send raw struct bytes
        bodySent = sendAll(
            client, reinterpret_cast<const char *>(&response),
            sizeof(response));
    }

    return bodySent;
}

ConnectionState handleVerify(SocketHandle client, Logger &logger) {
    ConnectionState result = ConnectionState::DISCONNECTED;

    PacketHeader header{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) — required to read raw struct bytes
    const bool headerOk = recvAll(
        client, reinterpret_cast<char *>(&header), sizeof(header));

    if (headerOk) {
        const std::string sender =
                "Aircraft-" + std::to_string(header.aircraftId);

        if (header.packetType != PacketType::VERIFY) {
            logger.logEvent(sender, "ERROR",
                            std::string("Expected VERIFY, got ") + //-V::2578
                            packetTypeName(header.packetType));
        } else if (header.payloadSize != sizeof(VerifyPayload)) {
            logger.logEvent(sender, "ERROR",
                            "VERIFY payload size mismatch");
        } else {
            VerifyPayload verifyData{};
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) — required to read raw struct bytes
            const bool payloadOk = recvAll(
                client, reinterpret_cast<char *>(&verifyData),
                sizeof(verifyData));

            if (payloadOk) {
                const uint32_t accepted =
                    (verifyData.protocolVersion == ATS_PROTOCOL_VERSION)
                    ? 1U : 0U;

                const bool sent = sendVerifyResponse(client, accepted);

                if (sent && (accepted == 1U)) {
                    logger.logEvent(sender, "STATE",
                                    "VERIFY handshake accepted");
                    result = ConnectionState::RECEIVING;
                } else if (sent) {
                    logger.logEvent(sender, "STATE",
                                    "VERIFY handshake rejected: "
                                    "protocol version mismatch");
                } else {
                    // Send failed — result remains DISCONNECTED
                }
            }
        }
    }

    return result;
}

/// Processes a single packet in the RECEIVING state. Returns true to
/// continue receiving, false to disconnect.
static bool processPacket(SocketHandle client, const PacketHeader &header,
                          Logger &logger) {
    bool continueRecv = true;
    const std::string sender =
            "Aircraft-" + std::to_string(header.aircraftId);

    if (header.packetType == PacketType::VERIFY) {
        logger.logEvent(sender, "ERROR",
                        "VERIFY not valid in RECEIVING state");
        continueRecv = false;
    } else if ((header.packetType == PacketType::TELEMETRY) &&
               (header.payloadSize == sizeof(TelemetryPayload))) {
        TelemetryPayload telemetry{};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) — required to read raw struct bytes
        const bool payloadOk = recvAll(
            client, reinterpret_cast<char *>(&telemetry),
            sizeof(telemetry));

        if (payloadOk) {
            const std::string msg =
                    std::string("TELEMETRY lat=") + //-V::2578
                    std::to_string(telemetry.latitude) +
                    " lon=" + std::to_string(telemetry.longitude) +
                    " alt=" + std::to_string(telemetry.altitude);

            logger.logEvent(sender, "INCOMING", msg);
        } else {
            continueRecv = false;
        }
    } else if (header.payloadSize > MAX_PAYLOAD_SIZE) {
        logger.logEvent(sender, "ERROR",
                        "Payload exceeds MAX_PAYLOAD_SIZE");
        continueRecv = false;
    } else {
        bool payloadOk = true;

        if (header.payloadSize > 0U) {
            std::array<char, MAX_PAYLOAD_SIZE> payload{};
            payloadOk = recvAll(client, payload.data(), header.payloadSize);
        }

        if (payloadOk) {
            const std::string msg =
                    std::string("Received ") + //-V::2578
                    packetTypeName(header.packetType) +
                    " payloadSize=" +
                    std::to_string(header.payloadSize);

            logger.logEvent(sender, "INCOMING", msg);
        } else {
            continueRecv = false;
        }
    }

    return continueRecv;
}

ConnectionState handleReceiving(SocketHandle client, Logger &logger) {
    const ConnectionState result = ConnectionState::DISCONNECTED;
    bool active = true;

    while (active && isRunning()) {
        PacketHeader header{};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) — required to read raw struct bytes
        const bool headerOk = recvAll(
            client, reinterpret_cast<char *>(&header), sizeof(header));

        if (!headerOk) {
            active = false;
        } else {
            active = processPacket(client, header, logger);
        }
    }

    return result;
}

void handleClient(SocketHandle client, Logger &logger) {
    logger.logEvent("Server", "STATE",
                    connectionStateName(ConnectionState::CONNECTED));

    ConnectionState state = handleVerify(client, logger);

    if (state == ConnectionState::RECEIVING) {
        logger.logEvent("Server", "STATE",
                        connectionStateName(ConnectionState::RECEIVING));
        state = handleReceiving(client, logger);
    }

    logger.logEvent("Server", "STATE",
                    connectionStateName(ConnectionState::DISCONNECTED));
    closeSocket(client);
}

// ---------------------------------------------------------------------------
// Server main loop
// ---------------------------------------------------------------------------

int runServer() {
    int result = 1;

    if (initPlatformSockets()) {
        // V2599: std::signal is banned by MISRA but no portable C++ alternative exists
        static_cast<void>(std::signal(SIGINT, signalHandler)); //-V::2599

        const uint16_t port = getPort();
        SocketHandle listenSock = createListenSocket(port);

        if (listenSock != INVALID_SOCK) {
            Logger logger;
            std::cout << "ATS server listening on port " << port << '\n';

            while (isRunning()) {
                SocketHandle client = accept(listenSock, nullptr, nullptr);
                if (client != INVALID_SOCK) {
                    handleClient(client, logger);
                }
            }

            std::cout << "\nShutting down server.\n";
            closeSocket(listenSock);
            result = 0;
        } else {
            std::cerr << "Failed to create listening socket on port "
                    << port << '\n';
        }

        cleanupPlatformSockets();
    } else {
        std::cerr << "Failed to initialize socket subsystem.\n";
    }

    return result;
}

} // namespace CSCN_74000_SERVER

