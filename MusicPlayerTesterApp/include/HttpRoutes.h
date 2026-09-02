/**
 * @file HttpRoutes.h
 * @author Soumyajit C
 * @brief Defines constants for HTTP methods, paths, default values, and query parameters.
 * @date 2026-09-02
 */

#pragma once

/**
 * @namespace HttpRoutes
 * @brief Contains constants for HTTP routing including methods, paths, defaults, and query parameters.
 *
 * This namespace organizes HTTP-related constants into sub-namespaces:
 * - Methods: Supported HTTP methods (GET, POST).
 * - Paths: Common server endpoints.
 * - Defaults: Default file names and response messages.
 * - QueryParams: Supported query string parameters.
 */
namespace HttpRoutes
{
    /**
     * @namespace Methods
     * @brief Supported HTTP methods.
     */
    namespace Methods
    {
        inline constexpr const char* get = "GET";   ///< HTTP GET method
        inline constexpr const char* post = "POST"; ///< HTTP POST method
    }

    /**
     * @namespace Paths
     * @brief Common server endpoints.
     */
    namespace Paths
    {
        inline constexpr const char* root = "/";             ///< Root path
        inline constexpr const char* index = "/index.html";  ///< Index page
        inline constexpr const char* play = "/play";         ///< Play endpoint
        inline constexpr const char* loop = "/loop";         ///< Loop endpoint
        inline constexpr const char* drumHit = "/drumHit";   ///< Drum hit endpoint
        inline constexpr const char* shutdown = "/shutdown"; ///< Shutdown endpoint
    }

    /**
     * @namespace Defaults
     * @brief Default file names and response messages.
     */
    namespace Defaults
    {
        inline constexpr const char* indexFile = "index.html"; ///< Default index file
        inline constexpr const char* responseOk = "OK";        ///< Default OK response
        inline constexpr const char* responseExit = "EXIT";    ///< Default EXIT response
    }

    /**
     * @namespace QueryParams
     * @brief Supported query string parameters.
     */
    namespace QueryParams
    {
        inline constexpr const char* instrument = "inst";   ///< Instrument parameter
        inline constexpr const char* frequency = "freq";    ///< Frequency parameter
        inline constexpr const char* volume = "vol";        ///< Volume parameter
        inline constexpr const char* duration = "dur";      ///< Duration parameter
        inline constexpr const char* action = "action";     ///< Action parameter
        inline constexpr const char* pattern = "pattern";   ///< Pattern parameter
        inline constexpr const char* bpm = "bpm";           ///< Beats per minute parameter
        inline constexpr const char* piece = "piece";       ///< Musical piece parameter
        inline constexpr const char* actionStart = "start"; ///< Action start value
    }
}
