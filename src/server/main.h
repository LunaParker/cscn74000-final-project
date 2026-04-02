#ifndef CSCN74000_PROJECT_SERVER_MAIN_H
#define CSCN74000_PROJECT_SERVER_MAIN_H

#include "common/logging.h"
#include "common/packets.h"
#include <cstdint>

namespace CSCN_74000_SERVER {

/// Default listening port when CSCN_74000_PORT is unset (port 34290)
constexpr uint16_t DEFAULT_PORT = 34290U;

/**
 * @brief Reads CSCN_74000_PORT from the environment, falling back to DEFAULT_PORT.
 * @return The port number the server should bind to.
 */
uint16_t getPort();

/**
 * @brief Performs platform-specific socket subsystem initialization.
 * @return true on success, false on failure.
 */
bool initPlatformSockets();

/**
 * @brief Tears down the platform-specific socket subsystem.
 */
void cleanupPlatformSockets();

/**
 * @brief Creates a TCP socket, binds it to the given port on INADDR_ANY, and
 *        begins listening for incoming connections.
 * @param port The port number to bind to.
 * @return A valid SocketHandle on success, or INVALID_SOCK on failure.
 */
SocketHandle createListenSocket(uint16_t port);

/**
 * @brief Closes a socket handle in a platform-agnostic manner.
 * @param sock The socket to close.
 */
void closeSocket(SocketHandle sock);

/**
 * @brief Receives exactly @p length bytes into @p buffer from @p sock.
 * @param sock   The connected socket to read from.
 * @param buffer Destination buffer (must be at least @p length bytes).
 * @param length Number of bytes to receive.
 * @return true if all bytes were received, false on error or disconnect.
 */
bool recvAll(SocketHandle sock, char* buffer, uint32_t length);

/**
 * @brief Sends exactly @p length bytes from @p buffer to @p sock.
 * @param sock   The connected socket to write to.
 * @param buffer Source buffer (must be at least @p length bytes).
 * @param length Number of bytes to send.
 * @return true if all bytes were sent, false on error or disconnect.
 */
bool sendAll(SocketHandle sock, const char* buffer, uint32_t length);

/**
 * @brief SIGINT handler — sets the shutdown flag so the accept loop exits.
 * @param signum The signal number (unused).
 */
void signalHandler(int signum);

/**
 * @brief Returns whether the server should continue running.
 * @return true while no shutdown signal has been received.
 */
bool isRunning();

/**
 * @brief Returns a human-readable string for a PacketType enum value.
 * @param type The packet type to convert.
 * @return A string literal naming the packet type, or "UNKNOWN".
 */
const char* packetTypeName(PacketType type);

/**
 * @brief Returns a human-readable string for a ConnectionState enum value.
 * @param state The connection state to convert.
 * @return A string literal naming the state, or "UNKNOWN".
 */
const char* connectionStateName(ConnectionState state);

/**
 * @brief Handles the VERIFYING state of the connection state machine.
 *
 * Reads a VERIFY packet from the client, validates it, and sends a
 * VerifyResponse back. Returns the next connection state.
 *
 * @param client The connected client socket handle.
 * @param logger The Logger instance used to record events.
 * @return RECEIVING on successful handshake, DISCONNECTED on failure.
 */
ConnectionState handleVerify(SocketHandle client, Logger& logger);

/**
 * @brief Handles the RECEIVING state of the connection state machine.
 *
 * Loops reading packets (TELEMETRY, FILE_METADATA, FILE_DATA) until
 * the client disconnects or an error occurs.
 *
 * @param client The connected client socket handle.
 * @param logger The Logger instance used to record events.
 * @return DISCONNECTED when the client disconnects or on error.
 */
ConnectionState handleReceiving(SocketHandle client, Logger& logger);

/**
 * @brief Handles a single accepted client connection.
 *
 * Drives the connection state machine through CONNECTED → VERIFYING →
 * RECEIVING → DISCONNECTED. Closes the client socket before returning.
 *
 * @param client The accepted client socket handle.
 * @param logger The Logger instance used to record events.
 */
void handleClient(SocketHandle client, Logger& logger);

/**
 * @brief Entry point for the ATS server.
 *
 * Initializes sockets, binds to the configured port, and loops accepting
 * one client at a time until SIGINT is received. Incoming packets are
 * logged via the Logger.
 *
 * @return 0 on clean shutdown, 1 on initialization failure.
 */
int runServer();

} // namespace CSCN_74000_SERVER

#endif
