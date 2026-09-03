/**
 * @file HttpServer.cpp
 * @author Soumyajit C
 * @brief Implementation of the HttpServer class.
 * @date 2026-09-02
 */

#include "HttpServer.h"
#include <iostream>

#if defined(_WIN32)
    #pragma comment(lib, "ws2_32.lib")
#endif

namespace
{
    constexpr int BufferSize = 2048; ///< Size of the receive buffer
}

HttpServer::HttpServer(int port, int backlog)
    : m_port(port), m_backlog(backlog)
{
}

HttpServer::~HttpServer()
{
    stop();
    cleanup();
}

bool HttpServer::start()
{
    if (m_running.load())
    {
        return true;
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "[HttpServer] Fatal: WSAStartup failed.\n";
        return false;
    }
    m_wsaInitialized = true;
#endif

    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == InvalidSocket)
    {
        std::cerr << "[HttpServer] Fatal: Socket allocation failed.\n";
        cleanup();
        return false;
    }

    int reuseAddr = 1;
#ifdef _WIN32
    setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuseAddr), sizeof(reuseAddr));
#else
    setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR,
               &reuseAddr, sizeof(reuseAddr));
#endif

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<uint16_t>(m_port));

    if (bind(m_listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SocketError)
    {
        std::cerr << "[HttpServer] Fatal: Bind failed on port " << m_port << ".\n";
        cleanup();
        return false;
    }

    if (listen(m_listenSocket, m_backlog) == SocketError)
    {
        std::cerr << "[HttpServer] Fatal: Listen failed.\n";
        cleanup();
        return false;
    }

    m_running.store(true);
    return true;
}

void HttpServer::run(const RequestHandler& handler)
{
    if (!m_running.load() || m_listenSocket == InvalidSocket)
    {
        return;
    }

    while (m_running.load())
    {
        socket_t clientSocket = accept(m_listenSocket, nullptr, nullptr);
        if (clientSocket == InvalidSocket)
        {
            if (!m_running.load())
            {
                break;
            }
            continue;
        }

        int nodelay = 1;
#ifdef _WIN32
        setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&nodelay), sizeof(nodelay));
#else
        setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY,
                   &nodelay, sizeof(nodelay));
#endif

        char buffer[BufferSize] = {0};
        int bytesReceived = static_cast<int>(recv(clientSocket, buffer, sizeof(buffer) - 1, 0));

        if (bytesReceived > 0)
        {
            std::string rawRequest(buffer, bytesReceived);
            std::string rawResponse;

            bool keepServerRunning = handler(rawRequest, rawResponse);

            if (!rawResponse.empty())
            {
                send(clientSocket, rawResponse.c_str(), static_cast<int>(rawResponse.length()), 0);
            }

            if (!keepServerRunning)
            {
                CloseSocketHandle(clientSocket);
                m_running.store(false);
                break;
            }
        }

        CloseSocketHandle(clientSocket);
    }
}

void HttpServer::stop()
{
    m_running.store(false);
    if (m_listenSocket != InvalidSocket)
    {
        CloseSocketHandle(m_listenSocket);
        m_listenSocket = InvalidSocket;
    }
}

void HttpServer::cleanup()
{
    stop();
#ifdef _WIN32
    if (m_wsaInitialized)
    {
        WSACleanup();
        m_wsaInitialized = false;
    }
#endif
}