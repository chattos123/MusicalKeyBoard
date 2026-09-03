/**
 * @file MusicPlayerSystemExport.h
 * @author Soumyajit C
 * @brief Export/import macro definitions for cross-platform dynamic library builds.
 * @date 2026-09-03
 *
 * This header defines the `MPS_API` macro used to control symbol visibility
 * when building or consuming the Music Player System library.
 *
 * - On Windows: uses `__declspec(dllexport)` when building the DLL and
 *   `__declspec(dllimport)` when consuming it.
 * - On POSIX systems (Linux, macOS): uses GCC/Clang visibility attributes.
 *
 * Usage:
 * - Annotate public classes, functions, or variables with `MPS_API` to ensure
 *   they are exported correctly from the shared library.
 */

#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef MUSIC_PLAYER_SYSTEM_EXPORTS
        /**
         * @def MPS_API
         * @brief Marks symbols for export when building the Music Player System DLL on Windows.
         */
        #define MPS_API __declspec(dllexport)
    #else
        /**
         * @def MPS_API
         * @brief Marks symbols for import when consuming the Music Player System DLL on Windows.
         */
        #define MPS_API __declspec(dllimport)
    #endif
#else
    /**
     * @def MPS_API
     * @brief Marks symbols with default visibility when building shared libraries on POSIX systems.
     */
    #define MPS_API __attribute__((visibility("default")))
#endif
