#include <iostream>
#include <memory>
#include <string>

#ifdef _WIN32
    #include "WindowsMusicSystem.h"
#else
    // Placeholder / interface implementation for Linux (e.g., ALSA or PulseAudio)
    #include "LinuxMusicSystem.h"
#endif

#include "DrumKit.h"
#include "ThreadPool.h"
#include "DrumLoopEngine.h"
#include "InstrumentManager.h"
#include "HttpServer.h"
#include "HttpUtils.h"
#include "HttpRequest.h"
#include "HttpRoutes.h"
#include "SocketPlatform.h"

namespace
{
    constexpr int ServerPort = 8080;
    constexpr size_t WorkerThreadCount = 8;
    constexpr const char* RootUrl = "http://localhost:8080";

    constexpr double DefaultFrequency = 261.63;
    constexpr double DefaultVolume = 0.8;
    constexpr double DefaultDuration = 1.2;
    constexpr double DefaultBpm = 110.0;
    constexpr double DrumVelocity = 0.9;
    constexpr double DrumDuration = 0.4;

    struct PlayTask
    {
        std::shared_ptr<IMusicInstrument> instrument;
        double frequency;
        double duration;
        double volume;

        void operator()() const
        {
            if (instrument)
            {
                instrument->PlayNote(frequency, duration, volume);
            }
        }
    };

    struct DrumHitTask
    {
        std::shared_ptr<DrumKit> drumKit;
        int pieceId;

        void operator()() const
        {
            if (drumKit)
            {
                drumKit->HitDrum(static_cast<DrumPiece>(pieceId), DrumVelocity, DrumDuration);
            }
        }
    };

    void ParsePlayParameters(const std::string& query, double& freq, double& vol, double& dur)
    {
        std::string freqStr = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::Frequency);
        std::string volStr = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::Volume);
        std::string durStr = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::Duration);

        if (!freqStr.empty()) freq = std::stod(freqStr);
        if (!volStr.empty())  vol  = std::stod(volStr);
        if (!durStr.empty())  dur  = std::stod(durStr);
    }

    void ParseLoopParameters(const std::string& query, std::string& pattern, double& bpm)
    {
        pattern = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::Pattern);
        std::string bpmStr = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::Bpm);
        if (!bpmStr.empty()) bpm = std::stod(bpmStr);
    }

    std::string MapPathToAssetFilename(const std::string& path)
    {
        if (path == HttpRoutes::Paths::Root || path == HttpRoutes::Paths::Index)
        {
            return HttpRoutes::Defaults::IndexFile;
        }
        if (!path.empty() && path.front() == '/')
        {
            return path.substr(1);
        }
        return path;
    }
}

int main()
{
    std::cout << "Starting Audio Subsystem & Synthesizer Station...\n";

#ifdef _WIN32
    auto audioSystem = std::make_shared<WindowsMusicSystem>();
#else
    auto audioSystem = std::make_shared<LinuxMusicSystem>();
#endif

    if (!audioSystem->Setup())
    {
        std::cerr << "Fatal: Failed to setup audio hardware subsystem.\n";
        return -1;
    }

    auto drumKit = std::make_shared<DrumKit>(audioSystem);
    DrumLoopEngine loopEngine(drumKit);
    ThreadPool audioPool(WorkerThreadCount);
    InstrumentManager instrumentManager(audioSystem);

    HttpServer server(ServerPort);
    if (!server.Start())
    {
        return -1;
    }

    std::cout << "Server running at: " << RootUrl << "\n";
    PlatformUtils::OpenBrowser(RootUrl);

    server.Run([&](const std::string& rawRequest, std::string& rawResponse) -> bool
    {
        HttpRequest request = HttpRequest::Parse(rawRequest);
        if (!request.isValid)
        {
            return true;
        }

        // 1. Play Note
        if (request.method == HttpRoutes::Methods::Get && request.path == HttpRoutes::Paths::Play)
        {
            std::string instName = HttpUtils::GetQueryParam(request.query, HttpRoutes::QueryParams::Instrument);
            double freq = DefaultFrequency;
            double vol = DefaultVolume;
            double dur = DefaultDuration;

            try { ParsePlayParameters(request.query, freq, vol, dur); } catch (...) {}

            std::shared_ptr<IMusicInstrument> instrument = instrumentManager.GetInstrument(instName);
            if (instrument)
            {
                audioPool.Enqueue(PlayTask{ instrument, freq, dur, vol });
            }

            rawResponse = HttpUtils::MakeHttpResponse(HttpUtils::Constants::MimePlain, 
                                                      HttpRoutes::Defaults::ResponseOk);
            return true;
        }

        // 2. Background Drum Loop
        if (request.method == HttpRoutes::Methods::Get && request.path == HttpRoutes::Paths::Loop)
        {
            std::string action = HttpUtils::GetQueryParam(request.query, HttpRoutes::QueryParams::Action);
            if (action == HttpRoutes::QueryParams::ActionStart)
            {
                std::string pattern;
                double bpm = DefaultBpm;
                try { ParseLoopParameters(request.query, pattern, bpm); } catch (...) {}
                loopEngine.Start(pattern, bpm);
            }
            else
            {
                loopEngine.Stop();
            }

            rawResponse = HttpUtils::MakeHttpResponse(HttpUtils::Constants::MimePlain, 
                                                      HttpRoutes::Defaults::ResponseOk);
            return true;
        }

        // 3. Single Drum Hit
        if (request.method == HttpRoutes::Methods::Get && request.path == HttpRoutes::Paths::DrumHit)
        {
            int pieceId = 0;
            try
            {
                std::string pieceStr = HttpUtils::GetQueryParam(request.query, HttpRoutes::QueryParams::Piece);
                if (!pieceStr.empty()) pieceId = std::stoi(pieceStr);
            }
            catch (...) {}

            audioPool.Enqueue(DrumHitTask{ drumKit, pieceId });
            rawResponse = HttpUtils::MakeHttpResponse(HttpUtils::Constants::MimePlain, 
                                                      HttpRoutes::Defaults::ResponseOk);
            return true;
        }

        // 4. Shutdown
        if ((request.method == HttpRoutes::Methods::Post || request.method == HttpRoutes::Methods::Get) 
            && request.path == HttpRoutes::Paths::Shutdown)
        {
            std::cout << "\n[Shutdown] Received exit signal from browser. Cleaning up...\n";
            rawResponse = HttpUtils::MakeHttpResponse(HttpUtils::Constants::MimePlain, 
                                                      HttpRoutes::Defaults::ResponseExit);
            return false;
        }

        // 5. Static Assets (HTML, CSS, JS, Locales)
        if (request.method == HttpRoutes::Methods::Get)
        {
            std::string targetFile = MapPathToAssetFilename(request.path);
            std::string fileContent;

            if (!targetFile.empty() && HttpUtils::TryLoadAsset(targetFile, fileContent))
            {
                rawResponse = HttpUtils::MakeHttpResponse(HttpUtils::GetMimeType(targetFile), fileContent);
            }
            else
            {
                rawResponse = HttpUtils::MakeHttpResponse(HttpUtils::Constants::MimeHtml,
                                                          HttpUtils::Constants::DefaultNotFoundBody,
                                                          HttpUtils::Constants::Status404);
            }
            return true;
        }

        return true;
    });

    loopEngine.Stop();
    server.Stop();
    audioSystem->Clear();

    std::cout << "[Shutdown] Audio subsystem and server stopped cleanly. Exiting application.\n";
    return 0;
}