/**
 * @file HttpUtils.h
 * @author Soumyajit C
 * @brief Utility functions and constants for HTTP response handling and asset management.
 * @date 2026-09-02
 */

#pragma once

#include <string>
#include <vector>

/**
 * @namespace HttpUtils
 * @brief Provides helper functions and constants for HTTP request/response handling.
 */
namespace HttpUtils
{
    /**
     * @namespace Constants
     * @brief Common HTTP status lines, headers, MIME types, and default response bodies.
     */
    namespace Constants
    {
        inline constexpr const char* Status200 = "HTTP/1.1 200 OK\r\n";              ///< HTTP 200 OK status line
        inline constexpr const char* Status404 = "HTTP/1.1 404 Not Found\r\n";       ///< HTTP 404 Not Found status line
        inline constexpr const char* HeaderCors = "Access-Control-Allow-Origin: *\r\n"; ///< CORS header
        inline constexpr const char* HeaderClose = "Connection: close\r\n\r\n";      ///< Connection close header

        inline constexpr const char* MimeHtml = "text/html; charset=utf-8";          ///< MIME type for HTML
        inline constexpr const char* MimeCss = "text/css";                           ///< MIME type for CSS
        inline constexpr const char* MimeJs = "application/javascript";              ///< MIME type for JavaScript
        inline constexpr const char* MimePlain = "text/plain";                       ///< MIME type for plain text
        inline constexpr const char* MimeOctetStream = "application/octet-stream";   ///< MIME type for binary data

        inline constexpr const char* DefaultNotFoundBody = "<h1>404 Not Found</h1>"; ///< Default 404 response body
    }

    /**
     * @brief Loads a file from disk into a string.
     * @param path [in] Path to the file.
     * @return File contents as a string, or empty string if not found.
     */
    std::string LoadFile(const std::string& path);

    /**
     * @brief Decodes a URL-encoded string.
     * @param str [in] URL-encoded string.
     * @return Decoded string.
     */
    std::string UrlDecode(const std::string& str);

    /**
     * @brief Extracts a query parameter value from a query string.
     * @param query [in] Full query string (e.g., "id=123&name=test").
     * @param key [in] Parameter key to search for.
     * @return Decoded parameter value, or empty string if not found.
     */
    std::string GetQueryParam(const std::string& query, const std::string& key);

    /**
     * @brief Determines MIME type based on file extension.
     * @param path [in] File path.
     * @return MIME type string.
     */
    std::string GetMimeType(const std::string& path);

    /**
     * @brief Resolves an asset path by searching common prefixes.
     * @param relativePath [in] Relative asset path.
     * @return Resolved path if found, otherwise empty string.
     */
    std::string ResolveAssetPath(const std::string& relativePath);

    /**
     * @brief Attempts to load an asset file.
     * @param relativePath [in] Relative asset path.
     * @param outContent [out] Loaded file content.
     * @return true if asset was successfully loaded, false otherwise.
     */
    bool TryLoadAsset(const std::string& relativePath, std::string& outContent);

    /**
     * @brief Formats and returns a raw HTTP response string.
     * @param contentType [in] MIME type of the response body.
     * @param body [in] Response body content.
     * @param statusLine [in] HTTP status line (default: 200 OK).
     * @return Complete HTTP response string.
     */
    std::string MakeHttpResponse(const std::string& contentType,
                                 const std::string& body,
                                 const std::string& statusLine = Constants::Status200);
}