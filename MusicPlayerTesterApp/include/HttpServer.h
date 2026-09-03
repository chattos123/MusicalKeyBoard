/**
 * @file HttpServer.h
 * @author Soumyajit C
 * @brief Declaration of the HttpServer class for handling basic HTTP requests.
 * @date 2026-09-02
 */

#pragma once

#include <string>
#include <functional>
#include <atomic>
#include "SocketPlatform.h"

/**
 * @class HttpServer
 * @brief A lightweight HTTP server implementation using sockets.
 *
 * The HttpServer class provides functionality to:
 * - Start and stop a TCP socket-based HTTP server.
 * - Accept incoming client connections.
 * - Process raw HTTP requests using a user-provided handler.
 * - Send responses back to clients.
 *
 * This class is non-copyable and manages socket cleanup internally.
 */
class HttpServer
{
public:
    /// Function type for handling HTTP requests.
    using RequestHandler = std::function<bool(const std::string& requestRaw, std::string& responseOut)>;

    /**
     * @brief Constructs an HttpServer instance.
     * @param port [in] Port number to bind the server (default: 8080).
     * @param backlog [in] Maximum number of pending connections (default: 10).
     */
    explicit HttpServer(int port = 8080, int backlog = 10);

    /**
     * @brief Destructor. Cleans up resources and stops the server if running.
     */
    ~HttpServer();

    // Deleted copy constructor and assignment operator
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    /**
     * @brief Starts the HTTP server.
     * @return true if the server started successfully, false otherwise.
     */
    bool start();

    /**
     * @brief Runs the server loop and processes incoming requests.
     * @param handler [in] Callback function to handle requests and generate responses.
     *
     * @remarks
     * - Accepts client connections in a loop until stopped.
     * - Calls the handler with raw request data and expects a response string.
     * - If the handler returns false, the server shuts down gracefully.
     */
    void run(const RequestHandler& handler);

    /**
     * @brief Stops the server and closes the listening socket.
     */
    void stop();

    /**
     * @brief Gets the port number the server is bound to.
     * @return Port number.
     */
    [[nodiscard]] int getPort() const { return m_port; }

    /**
     * @brief Checks if the server is currently running.
     * @return true if running, false otherwise.
     */
    [[nodiscard]] bool isRunning() const { return m_running.load(); }

private:
    /// Cleans up socket resources and resets state.
    void cleanup();

    int m_port;                          ///< Port number
    int m_backlog;                       ///< Connection backlog
    socket_t m_listenSocket{InvalidSocket}; ///< Listening socket handle
    std::atomic<bool> m_running{false};  ///< Server running state
#ifdef _WIN32
    bool m_wsaInitialized{false};        ///< Tracks WSA initialization on Windows
#endif
};