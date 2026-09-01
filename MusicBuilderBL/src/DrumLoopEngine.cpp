#include "DrumLoopEngine.h"
#include <chrono>

DrumLoopEngine::DrumLoopEngine(std::shared_ptr<DrumKit> drums)
    : m_drums(std::move(drums)) {}

DrumLoopEngine::~DrumLoopEngine() 
{
    Stop();
}

void DrumLoopEngine::Start(const std::string& pattern, double bpm) 
{
    Stop();
    if (bpm < 40.0) bpm = 40.0;
    if (bpm > 240.0) bpm = 240.0;

    m_running.store(true);
    m_loopThread = std::thread(&DrumLoopEngine::LoopWorker, this, pattern, bpm);
}

void DrumLoopEngine::Stop() 
{
    m_running.store(false);

    if (m_loopThread.joinable()) 
    {
        m_loopThread.join();
    }
}

void DrumLoopEngine::LoopWorker(std::string pattern, double bpm) 
{
    double beatIntervalMs = (60.0 / bpm) * 1000.0;
    int step = 0;

    while (m_running.load()) 
    {
        auto start = std::chrono::steady_clock::now();

        if (pattern == "RockBeat") 
        {
            if (step == 0 || step == 2) 
            {
                m_drums->PlayBeat({ DrumPiece::BassDrum, DrumPiece::ClosedHiHat }, 0.25, 0.9);
            } 
            else 
            {
                m_drums->PlayBeat({ DrumPiece::SnareDrum, DrumPiece::ClosedHiHat }, 0.25, 0.85);
            }
            step = (step + 1) % 4;
        } 
        else if (pattern == "Metronome") 
        {
            if (step == 0) {
                m_drums->PlayBeat({ DrumPiece::BassDrum }, 0.15, 0.8);
            } else {
                m_drums->PlayBeat({ DrumPiece::ClosedHiHat }, 0.15, 0.6);
            }
            step = (step + 1) % 4;
        }
        else 
        { // FunkBeat (16-step)
            if (step == 0) m_drums->PlayBeat({ DrumPiece::BassDrum, DrumPiece::ClosedHiHat }, 0.15, 0.9);
            else if (step == 4 || step == 12) m_drums->PlayBeat({ DrumPiece::SnareDrum, DrumPiece::ClosedHiHat }, 0.15, 0.85);
            else if (step == 6 || step == 10) m_drums->PlayBeat({ DrumPiece::BassDrum }, 0.15, 0.8);
            else m_drums->PlayBeat({ DrumPiece::ClosedHiHat }, 0.15, 0.5);

            step = (step + 1) % 16;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        double waitTime = beatIntervalMs - elapsed;
        int waitSteps = static_cast<int>(waitTime / 10.0);
        
        for (int i = 0; i < waitSteps && m_running.load(); ++i) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}