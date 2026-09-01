#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef MUSIC_INSTRUMENT_EXPORTS
        #define MI_API __declspec(dllexport)
    #else
        #define MI_API __declspec(dllimport)
    #endif
#else
    #define MI_API __attribute__((visibility("default")))
#endif