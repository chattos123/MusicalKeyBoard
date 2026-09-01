#pragma once

#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include "DrumKit.h"

class DrumLoopEngine 
{
private:
    std::shared_ptr<DrumKit> m_drums;
    std::atomic<bool> m_running{ false };
    std::thread m_loopThread;

    void LoopWorker(std::string pattern, double bpm);

public:
    explicit DrumLoopEngine(std::shared_ptr<DrumKit> drums);
    ~DrumLoopEngine();

    void Start(const std::string& pattern, double bpm);
    void Stop();
};