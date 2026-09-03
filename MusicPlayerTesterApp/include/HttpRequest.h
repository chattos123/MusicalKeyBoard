/**
 * @file HttpRequest.h
 * @author Soumyajit C
 * @brief Declaration of the HttpRequest struct and its parsing function.
 * @date 2026-09-02
 */

#pragma once

#include <string>

/**
 * @class HttpRequest
 * @brief Represents a parsed HTTP request line.
 *
 * The HttpRequest struct stores the method, path, query string, and version
 * of an HTTP request. It provides a static function to parse a raw request line
 * into its components.
 */
struct HttpRequest
{
    std::string method;    ///< HTTP method (e.g., GET, POST)
    std::string path;      ///< Request path (e.g., /index.html)
    std::string query;     ///< Query string (e.g., id=123&name=test)
    std::string version;   ///< HTTP version (e.g., HTTP/1.1)
    bool isValid{false};   ///< Indicates whether parsing was successful

    /**
     * @brief Parses a raw HTTP request line into an HttpRequest object.
     *
     * @param rawRequest [in] The raw HTTP request line as a string.
     * @return HttpRequest [out] Parsed HttpRequest object with method, path, query, and version.
     *
     * @remarks
     * - Strips trailing carriage return (`\r`) if present.
     * - Splits the request line into method, target (path + query), and version.
     * - Extracts query parameters if a `?` is found in the target.
     * - Sets `isValid` to true if parsing succeeds, false otherwise.
     */
    static HttpRequest parse(const std::string& rawRequest);
};
