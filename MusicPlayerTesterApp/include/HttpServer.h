#pragma once

#include <string>
#include <functional>
#include <atomic>
#include "SocketPlatform.h"

class HttpServer
{
public:
    using RequestHandler = std::function<bool(const std::string& requestRaw, std::string& responseOut)>;

    explicit HttpServer(int port = 8080, int backlog = 10);
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    bool Start();
    void Run(const RequestHandler& handler);
    void Stop();

    [[nodiscard]] int GetPort() const { return m_port; }
    [[nodiscard]] bool IsRunning() const { return m_running.load(); }

private:
    void Cleanup();

    int m_port;
    int m_backlog;
    socket_t m_listenSocket{ InvalidSocket };
    std::atomic<bool> m_running{ false };
#ifdef _WIN32
    bool m_wsaInitialized{ false };
#endif
};