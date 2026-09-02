#pragma once

#include <string>
#include <vector>

namespace HttpUtils
{
    namespace Constants
    {
        inline constexpr const char* Status200 = "HTTP/1.1 200 OK\r\n";
        inline constexpr const char* Status404 = "HTTP/1.1 404 Not Found\r\n";
        inline constexpr const char* HeaderCors = "Access-Control-Allow-Origin: *\r\n";
        inline constexpr const char* HeaderClose = "Connection: close\r\n\r\n";

        inline constexpr const char* MimeHtml = "text/html; charset=utf-8";
        inline constexpr const char* MimeCss = "text/css";
        inline constexpr const char* MimeJs = "application/javascript";
        inline constexpr const char* MimePlain = "text/plain";
        inline constexpr const char* MimeOctetStream = "application/octet-stream";

        inline constexpr const char* DefaultNotFoundBody = "<h1>404 Not Found</h1>";
    }

    std::string LoadFile(const std::string& path);
    std::string UrlDecode(const std::string& str);
    std::string GetQueryParam(const std::string& query, const std::string& key);
    std::string GetMimeType(const std::string& path);
    std::string ResolveAssetPath(const std::string& relativePath);
    bool TryLoadAsset(const std::string& relativePath, std::string& outContent);

    // Formats and returns a raw HTTP response string
    std::string MakeHttpResponse(const std::string& contentType, 
                                 const std::string& body, 
                                 const std::string& statusLine = Constants::Status200);
}