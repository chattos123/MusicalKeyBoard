/**
 * @file SocketPlatform.h
 * @author Soumyajit C
 * @brief Cross-platform socket type definitions and utility functions.
 * @date 2026-09-02
 *
 * This header provides platform-independent socket type aliases and
 * utility functions for closing sockets. It also includes a helper
 * function to open a web browser with a given URL across Windows,
 * macOS, and Linux.
 */

#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <shellapi.h>

    /// Socket type alias for Windows
    using socket_t = SOCKET;

    /// Invalid socket constant for Windows
    inline constexpr socket_t InvalidSocket = INVALID_SOCKET;

    /// Socket error constant for Windows
    inline constexpr int SocketError = SOCKET_ERROR;

    /**
     * @brief Closes a socket handle on Windows.
     * @param sock [in] Socket handle to close.
     * @return int Result of closesocket().
     */
    inline int CloseSocketHandle(socket_t sock) 
    {
        return closesocket(sock);
    }

#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <cstdlib>

    /// Socket type alias for POSIX systems
    using socket_t = int;

    /// Invalid socket constant for POSIX
    inline constexpr socket_t InvalidSocket = -1;

    /// Socket error constant for POSIX
    inline constexpr int SocketError = -1;

    /**
     * @brief Closes a socket handle on POSIX systems.
     * @param sock [in] Socket handle to close.
     * @return int Result of close().
     */
    inline int CloseSocketHandle(socket_t sock) 
    {
        return close(sock);
    }
#endif

/**
 * @namespace PlatformUtils
 * @brief Provides platform-specific utility functions.
 */
namespace PlatformUtils
{
    /**
     * @brief Opens the default web browser with the given URL.
     *
     * @param url [in] The URL to open.
     *
     * @remarks
     * - On Windows, uses ShellExecuteA.
     * - On macOS, uses the `open` command.
     * - On Linux, uses `xdg-open`.
     */
    inline void OpenBrowser(const std::string& url)
    {
#ifdef _WIN32
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
        std::string cmd = "open " + url;
        (void)std::system(cmd.c_str());
#else
        std::string cmd = "xdg-open " + url + " > /dev/null 2>&1 &";
        (void)std::system(cmd.c_str());
#endif
    }
}