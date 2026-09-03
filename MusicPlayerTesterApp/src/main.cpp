/**
 * @file main.cpp
 * @author Soumyajit C
 * @brief Entry point for the Synthesizer Station application.
 * @date 2026-09-02
 */

#include <iostream>
#include <memory>
#include <string>

#ifdef _WIN32
    #include "WindowsMusicSystem.h"
#else
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

    /**
     * @struct PlayTask
     * @brief Represents a task to play a musical note asynchronously.
     *
     * Members:
     * - instrument: Instrument to play the note on.
     * - frequency: Frequency of the note.
     * - duration: Duration of the note.
     * - volume: Volume of the note.
     *
     * Operator():
     * - Plays the note if the instrument is valid.
     */
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

     /**
     * @struct DrumHitTask
     * @brief Represents a task to hit a drum asynchronously.
     *
     * Members:
     * - drumKit: Drum kit to hit.
     * - pieceId: ID of the drum piece.
     *
     * Operator():
     * - Hits the specified drum piece if drumKit is valid.
     */
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

      /**
     * @brief Parses play parameters (frequency, volume, duration) from query string.
     * @param query [in] Query string containing parameters.
     * @param freq [out] Frequency value.
     * @param vol [out] Volume value.
     * @param dur [out] Duration value.
     */
    void ParsePlayParameters(const std::string& query, double& freq, double& vol, double& dur)
    {
        std::string freqStr = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::frequency);
        std::string volStr  = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::volume);
        std::string durStr  = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::duration);

        if (!freqStr.empty()) freq = std::stod(freqStr);
        if (!volStr.empty())  vol  = std::stod(volStr);
        if (!durStr.empty())  dur  = std::stod(durStr);
    }

    /**
     * @brief Parses loop parameters (pattern, bpm) from query string.
     * @param query [in] Query string containing parameters.
     * @param pattern [out] Drum loop pattern string.
     * @param bpm [out] Beats per minute value.
     */
    void ParseLoopParameters(const std::string& query, std::string& pattern, double& bpm)
    {
        pattern = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::pattern);
        std::string bpmStr = HttpUtils::GetQueryParam(query, HttpRoutes::QueryParams::bpm);
        if (!bpmStr.empty()) bpm = std::stod(bpmStr);
    }

     /**
     * @brief Maps HTTP path to asset filename.
     * @param path [in] HTTP request path.
     * @return Resolved asset filename.
     */
    std::string MapPathToAssetFilename(const std::string& path)
    {
        if (path == HttpRoutes::Paths::root || path == HttpRoutes::Paths::index)
        {
            return HttpRoutes::Defaults::indexFile;
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
    if (!server.start())
    {
        return -1;
    }

    std::cout << "Server running at: " << RootUrl << "\n";
    PlatformUtils::OpenBrowser(RootUrl);

    server.run([&](const std::string& rawRequest, std::string& rawResponse) -> bool
    {
        HttpRequest request = HttpRequest::parse(rawRequest);
        if (!request.isValid)
        {
            return true;
        }

        // 1. Play Note
        if (request.method == HttpRoutes::Methods::get && request.path == HttpRoutes::Paths::play)
        {
            std::string instName = HttpUtils::GetQueryParam(request.query, HttpRoutes::QueryParams::instrument);
            double freq = DefaultFrequency;
            double vol  = DefaultVolume;
            double dur  = DefaultDuration;

            try { ParsePlayParameters(request.query, freq, vol, dur); } catch (...) {}

            std::shared_ptr<IMusicInstrument> instrument = instrumentManager.GetInstrument(instName);
            if (instrument)
            {
                audioPool.Enqueue(PlayTask{ instrument, freq, dur, vol });
            }

            rawResponse = HttpUtils::MakeHttpResponse(HttpUtils::Constants::MimePlain,
                                                      HttpRoutes::Defaults::responseOk);
            return true;
        }

        // 2. Background Drum Loop
        if (request.method == HttpRoutes::Methods::get && request.path == HttpRoutes::Paths::loop)
        {
            std::string action = HttpUtils::GetQueryParam(request.query, HttpRoutes::QueryParams::action);
            if (action == HttpRoutes::QueryParams::actionStart)
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
                                                      HttpRoutes::Defaults::responseOk);
            return true;
        }

        // 3. Single Drum Hit
        if (request.method == HttpRoutes::Methods::get && request.path == HttpRoutes::Paths::drumHit)
        {
            int pieceId = 0;
            try
            {
                std::string pieceStr = HttpUtils::GetQueryParam(request.query, HttpRoutes::QueryParams::piece);
                if (!pieceStr.empty()) pieceId = std::stoi(pieceStr);
            }
            catch (...) {}

            audioPool.Enqueue(DrumHitTask{ drumKit, pieceId });
            rawResponse = HttpUtils::MakeHttpResponse(HttpUtils::Constants::MimePlain,
                                                      HttpRoutes::Defaults::responseOk);
            return true;
        }

        // 4. Shutdown
        if ((request.method == HttpRoutes::Methods::post || request.method == HttpRoutes::Methods::get)
            && request.path == HttpRoutes::Paths::shutdown)
        {
            std::cout << "\n[Shutdown] Received exit signal from browser. Cleaning up...\n";
            rawResponse = HttpUtils::MakeHttpResponse(HttpUtils::Constants::MimePlain,
                                                      HttpRoutes::Defaults::responseExit);
            return false;
        }

        // 5. Static Assets
        if (request.method == HttpRoutes::Methods::get)
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
    server.stop();
    audioSystem->Clear();

    std::cout << "[Shutdown] Audio subsystem and server stopped cleanly. Exiting application.\n";
    return 0;
}