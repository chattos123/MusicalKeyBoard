#include "Saxophone.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

Saxophone::Saxophone(std::shared_ptr<IMusicSystem> system)
    : m_system(std::move(system)) 
{
}

std::string Saxophone::GetName() const 
{
    return "Saxophone";
}

void Saxophone::SetMute(BrassMuteType mute) 
{
    m_currentMute = mute;
}

void Saxophone::PlayChord(const std::vector<double>& frequencies, double duration, double velocity) 
{
    for (double f : frequencies) 
    {
        PlayNote(f, duration, velocity * 0.7);
    }
}

void Saxophone::Tonguing(double frequencyHz, int noteCount, double noteDuration, double velocity) 
{
    for (int i = 0; i < noteCount; ++i) 
    {
        PlayNote(frequencyHz, noteDuration * 0.85, velocity);
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(noteDuration * 1000.0)));
    }
}

void Saxophone::PlayNote(double frequency, double duration, double velocity) 
{
    if (!m_system || frequency <= 0.0 || duration <= 0.0) return;

    double sampleRate = m_system->GetSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;

    size_t totalSamples = static_cast<size_t>(sampleRate * duration);
    std::vector<float> buffer(totalSamples, 0.0f);

    const double twoPi = 6.283185307179586;
    double phase = 0.0;
    double phaseInc = (twoPi * frequency) / sampleRate;

    for (size_t i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;

        // Linear envelope fade in/out to guarantee audible sound without clicks
        double env = 1.0;
        if (t < 0.05) 
        {
            env = t / 0.05;
        } 
        else if (t > (duration - 0.08)) 
        {
            env = (duration - t) / 0.08;
        }

        phase += phaseInc;
        if (phase >= twoPi) phase -= twoPi;

        // Rich reed harmonics: 1st, 2nd, 3rd, 4th, 5th
        double s = std::sin(phase)
                 + 0.50 * std::sin(2.0 * phase)
                 + 0.35 * std::sin(3.0 * phase)
                 + 0.15 * std::sin(4.0 * phase);

        buffer[i] = static_cast<float>(std::clamp(s * env * velocity * 0.5, -1.0, 1.0));
    }

    m_system->MixAudioAsync(buffer);
}