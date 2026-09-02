#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <shellapi.h>

    using socket_t = SOCKET;
    inline constexpr socket_t InvalidSocket = INVALID_SOCKET;
    inline constexpr int SocketError = SOCKET_ERROR;

    inline int CloseSocketHandle(socket_t sock) {
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

    using socket_t = int;
    inline constexpr socket_t InvalidSocket = -1;
    inline constexpr int SocketError = -1;

    inline int CloseSocketHandle(socket_t sock) {
        return close(sock);
    }
#endif

namespace PlatformUtils
{
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