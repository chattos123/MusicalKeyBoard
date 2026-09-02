#pragma once

namespace HttpRoutes
{
    namespace Methods
    {
        inline constexpr const char* Get = "GET";
        inline constexpr const char* Post = "POST";
    }

    namespace Paths
    {
        inline constexpr const char* Root = "/";
        inline constexpr const char* Index = "/index.html";
        inline constexpr const char* Play = "/play";
        inline constexpr const char* Loop = "/loop";
        inline constexpr const char* DrumHit = "/drumHit";
        inline constexpr const char* Shutdown = "/shutdown";
    }

    namespace Defaults
    {
        inline constexpr const char* IndexFile = "index.html";
        inline constexpr const char* ResponseOk = "OK";
        inline constexpr const char* ResponseExit = "EXIT";
    }

    namespace QueryParams
    {
        inline constexpr const char* Instrument = "inst";
        inline constexpr const char* Frequency = "freq";
        inline constexpr const char* Volume = "vol";
        inline constexpr const char* Duration = "dur";
        inline constexpr const char* Action = "action";
        inline constexpr const char* Pattern = "pattern";
        inline constexpr const char* Bpm = "bpm";
        inline constexpr const char* Piece = "piece";
        inline constexpr const char* ActionStart = "start";
    }
}