#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <winsock2.h>
#include <windows.h>
#include <shellapi.h>

#include "WindowsMusicSystem.h"
#include "Guitar.h"
#include "BassGuitar.h"
#include "Mandolin.h"
#include "Violin.h"
#include "Trumpet.h"
#include "Kalimba.h"
#include "Piano.h"
#include "DrumKit.h"
#include "Harmonium.h"
#include "Saxophone.h"

#pragma comment(lib, "ws2_32.lib")

class ThreadPool {
private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stop{ false };

public:
    explicit ThreadPool(size_t threads = 8) {
        for (size_t i = 0; i < threads; ++i) {
            m_workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(m_queueMutex);
                        m_cv.wait(lock, [this]() {
                            return m_stop.load() || !m_tasks.empty();
                        });

                        if (m_stop.load() && m_tasks.empty()) return;

                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        m_stop.store(true);
        m_cv.notify_all();
        for (std::thread& worker : m_workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void Enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_tasks.push(std::move(task));
        }
        m_cv.notify_one();
    }
};

class DrumLoopEngine {
private:
    std::shared_ptr<DrumKit> m_drums;
    std::atomic<bool> m_running{ false };
    std::thread m_loopThread;

public:
    explicit DrumLoopEngine(std::shared_ptr<DrumKit> drums) : m_drums(std::move(drums)) {}

    ~DrumLoopEngine() {
        Stop();
    }

    void Start(const std::string& pattern, double bpm) {
        Stop();
        if (bpm < 40.0) bpm = 40.0;
        if (bpm > 240.0) bpm = 240.0;

        m_running.store(true);
        m_loopThread = std::thread(&DrumLoopEngine::LoopWorker, this, pattern, bpm);
    }

    void Stop() {
        m_running.store(false);
        if (m_loopThread.joinable()) {
            m_loopThread.join();
        }
    }

private:
    void LoopWorker(std::string pattern, double bpm) {
        double beatIntervalMs = (60.0 / bpm) * 1000.0;
        int step = 0;

        while (m_running.load()) {
            auto start = std::chrono::steady_clock::now();

            if (pattern == "RockBeat") {
                if (step == 0 || step == 2) {
                    m_drums->PlayBeat({ DrumPiece::BassDrum, DrumPiece::ClosedHiHat }, 0.25, 0.9);
                } else {
                    m_drums->PlayBeat({ DrumPiece::SnareDrum, DrumPiece::ClosedHiHat }, 0.25, 0.85);
                }
                step = (step + 1) % 4;
            } else if (pattern == "Metronome") {
                if (step == 0) {
                    m_drums->PlayBeat({ DrumPiece::BassDrum }, 0.15, 0.8);
                } else {
                    m_drums->PlayBeat({ DrumPiece::ClosedHiHat }, 0.15, 0.6);
                }
                step = (step + 1) % 4;
            } else { // FunkBeat (16-step)
                if (step == 0) m_drums->PlayBeat({ DrumPiece::BassDrum, DrumPiece::ClosedHiHat }, 0.15, 0.9);
                else if (step == 4 || step == 12) m_drums->PlayBeat({ DrumPiece::SnareDrum, DrumPiece::ClosedHiHat }, 0.15, 0.85);
                else if (step == 6 || step == 10) m_drums->PlayBeat({ DrumPiece::BassDrum }, 0.15, 0.8);
                else m_drums->PlayBeat({ DrumPiece::ClosedHiHat }, 0.15, 0.5);

                step = (step + 1) % 16;
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
            double waitTime = beatIntervalMs - elapsed;

            // Small granular sleeps allow instant termination when Stop() is called
            int waitSteps = static_cast<int>(waitTime / 10.0);
            for (int i = 0; i < waitSteps && m_running.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
};

std::string LoadHtmlFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "<h1>index.html not found! Place it in the project root.</h1>";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string GetQueryParam(const std::string& query, const std::string& key) {
    size_t pos = query.find(key + "=");
    if (pos == std::string::npos) return "";
    pos += key.length() + 1;
    size_t end = query.find('&', pos);
    if (end == std::string::npos) end = query.length();
    return query.substr(pos, end - pos);
}

int main()
{
    std::cout << "Starting Audio Subsystem & Synthesizer Station...\n";

    auto audioSystem = std::make_shared<WindowsMusicSystem>();
    if (!audioSystem->Setup()) {
        std::cerr << "Fatal: Failed to setup Windows Audio System.\n";
        return -1;
    }

    auto drumKit = std::make_shared<DrumKit>(audioSystem);
    DrumLoopEngine loopEngine(drumKit);

    // Dedicated worker pool to eliminate thread creation latency
    ThreadPool audioPool(8);

    std::unordered_map<std::string, std::shared_ptr<IMusicInstrument>> instruments;
    instruments["Piano"] = std::make_shared<Piano>(audioSystem);
    instruments["Guitar"] = std::make_shared<Guitar>(audioSystem);
    instruments["BassGuitar"] = std::make_shared<BassGuitar>(audioSystem);
    instruments["Mandolin"] = std::make_shared<Mandolin>(audioSystem);
    instruments["Violin"] = std::make_shared<Violin>(audioSystem);
    instruments["Trumpet"] = std::make_shared<Trumpet>(audioSystem);
    instruments["Kalimba"] = std::make_shared<Kalimba>(audioSystem);
    instruments["Harmonium"] = std::make_shared<Harmonium>(audioSystem, true);

    auto sax = instruments["Saxophone"] = std::make_shared<Saxophone>(audioSystem);
    instruments["Saxophone"] = sax;
    instruments["Alto Saxophone"] = sax;
    instruments["AltoSaxophone"] = sax;
    instruments["Alto%20Saxophone"] = sax;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 10);

    std::cout << "Server running at: http://localhost:8080\n";
    ShellExecuteA(NULL, "open", "http://localhost:8080", NULL, NULL, SW_SHOWNORMAL);

    std::string htmlContent = LoadHtmlFile("../../index.html");

    if (htmlContent.find("not found") != std::string::npos) 
    {
        htmlContent = LoadHtmlFile("index.html");
    }

    bool running = true;

    while (running) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) continue;

        // Disable TCP buffering delay
        int nodelay = 1;
        setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));

        char buffer[2048] = { 0 };
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0) {
            std::string request(buffer);

            // 1. Serve HTML5 Frontend
            if (request.find("GET / ") == 0 || request.find("GET /index.html") == 0) {
                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                    std::to_string(htmlContent.length()) + "\r\n\r\n" + htmlContent;
                send(clientSocket, response.c_str(), (int)response.length(), 0);
            }
            // 2. Melodic Keyboard Trigger
            else if (request.find("GET /play?") == 0) {
                size_t start = request.find("GET /play?") + 10;
                size_t end = request.find(" HTTP/");
                std::string query = request.substr(start, end - start);

                std::string inst = GetQueryParam(query, "inst");
                double freq = 261.63, vol = 0.8, dur = 1.2;
                try {
                    std::string fStr = GetQueryParam(query, "freq");
                    std::string vStr = GetQueryParam(query, "vol");
                    std::string dStr = GetQueryParam(query, "dur");
                    if (!fStr.empty()) freq = std::stod(fStr);
                    if (!vStr.empty()) vol = std::stod(vStr);
                    if (!dStr.empty()) dur = std::stod(dStr);
                } catch (...) {}

                if (instruments.find(inst) != instruments.end()) {
                    audioPool.Enqueue([=]() {
                        instruments.at(inst)->PlayNote(freq, dur, vol);
                    });
                }

                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 2\r\n\r\nOK";
                send(clientSocket, response.c_str(), (int)response.length(), 0);
            }
            // 3. Background Drum Loop Control
            else if (request.find("GET /loop?") == 0) {
                size_t start = request.find("GET /loop?") + 10;
                size_t end = request.find(" HTTP/");
                std::string query = request.substr(start, end - start);

                std::string action = GetQueryParam(query, "action");
                if (action == "start") {
                    std::string pattern = GetQueryParam(query, "pattern");
                    double bpm = 110.0;
                    try {
                        std::string bpmStr = GetQueryParam(query, "bpm");
                        if (!bpmStr.empty()) bpm = std::stod(bpmStr);
                    } catch (...) {}
                    loopEngine.Start(pattern, bpm);
                } else {
                    loopEngine.Stop();
                }

                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 2\r\n\r\nOK";
                send(clientSocket, response.c_str(), (int)response.length(), 0);
            }
            // 4. Single Drum Hit Pad
            else if (request.find("GET /drumHit?") == 0) {
                size_t start = request.find("GET /drumHit?") + 13;
                size_t end = request.find(" HTTP/");
                std::string query = request.substr(start, end - start);

                int pieceId = 0;
                try {
                    std::string pStr = GetQueryParam(query, "piece");
                    if (!pStr.empty()) pieceId = std::stoi(pStr);
                } catch (...) {}

                audioPool.Enqueue([=]() {
                    drumKit->HitDrum(static_cast<DrumPiece>(pieceId), 0.9, 0.4);
                });

                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 2\r\n\r\nOK";
                send(clientSocket, response.c_str(), (int)response.length(), 0);
            }
            // 5. Shutdown Endpoint
            else if (request.find("POST /shutdown") == 0 || request.find("GET /shutdown") == 0) {
                std::cout << "\n[Shutdown] Received exit signal from browser. Cleaning up...\n";
                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 4\r\n\r\nEXIT";
                send(clientSocket, response.c_str(), (int)response.length(), 0);
                closesocket(clientSocket);
                running = false;
                break;
            }
        }
        closesocket(clientSocket);
    }

    loopEngine.Stop();
    closesocket(serverSocket);
    WSACleanup();
    audioSystem->Clear();

    std::cout << "[Shutdown] Audio subsystem and server stopped cleanly. Exiting application.\n";
    return 0;
}