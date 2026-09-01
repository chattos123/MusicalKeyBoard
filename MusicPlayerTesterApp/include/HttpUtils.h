#pragma once

#include <string>
#include <winsock2.h>

namespace HttpUtils 
{
    std::string LoadHtmlFile(const std::string& path);
    std::string UrlDecode(const std::string& str);
    std::string GetQueryParam(const std::string& query, const std::string& key);
    std::string ResolveHtmlContent();
    void SendHttpResponse(SOCKET clientSocket, const std::string& contentType, const std::string& body);
}