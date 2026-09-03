/**
 * @file MusicInstrumentExport.h
 * @brief Dynamic library symbol visibility and export macro definitions for MusicInstrument.
 * @details Configures cross-platform DLL symbol import/export linkage directives for 
 *          Windows (MSVC, MinGW, Cygwin) and default symbol visibility attributes for 
 *          POSIX/Linux compilers (GCC, Clang).
 * 
 * @author Soumyajit Chatterjee
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef MUSIC_INSTRUMENT_EXPORTS
        #define MI_API __declspec(dllexport)
    #else
        #define MI_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define MI_API __attribute__((visibility("default")))
#else
    #define MI_API
#endif