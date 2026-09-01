#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef MUSIC_PLAYER_SYSTEM_EXPORTS
        #define MPS_API __declspec(dllexport)
    #else
        #define MPS_API __declspec(dllimport)
    #endif
#else
    #define MPS_API __attribute__((visibility("default")))
#endif