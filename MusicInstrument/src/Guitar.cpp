#include "Guitar.h"
#include <cmath>
#include <random>
#include <algorithm>

Guitar::Guitar(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)), m_name("Acoustic Steel-String Guitar"),
      m_openTuning{ 82.41, 110.00, 146.83, 196.00, 246.94, 329.63 } {}

std::string Guitar::GetName() const 
{
    return m_name;
}

void Guitar::PlayNote(double frequencyHz, double durationSeconds, double velocity) 
{
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

void Guitar::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) 
{
    Strum(frequencies, 0.0, durationSeconds);
}

int Guitar::GetStringCount() const 
{
    return static_cast<int>(m_openTuning.size());
}

void Guitar::PluckString(int stringIndex, double durationSeconds, double velocity) 
{
    if (stringIndex >= 0 && stringIndex < static_cast<int>(m_openTuning.size())) 
    {
        PlayNote(m_openTuning[stringIndex], durationSeconds, velocity);
    }
}

void Guitar::Strum(const std::vector<double>& chordFrequencies, double strumTimeMs, double durationSeconds) 
{
    if (chordFrequencies.empty() || !m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (size_t i = 0; i < chordFrequencies.size(); ++i) 
    {
        double freq = chordFrequencies[i];
        size_t delayOffset = static_cast<size_t>((i * strumTimeMs / 1000.0) * sampleRate);
        size_t ringSize = static_cast<size_t>(sampleRate / freq);
        if (ringSize < 2) ringSize = 2;

        std::vector<float> ring(ringSize);
        for (size_t k = 0; k < ringSize; ++k) ring[k] = dist(rng);

        size_t idx = 0;
        const float decay = 0.996f;

        for (size_t n = delayOffset; n < totalSamples; ++n) 
        {
            float currentVal = ring[idx];
            size_t nextIdx = (idx + 1) % ringSize;
            ring[idx] = decay * 0.5f * (ring[idx] + ring[nextIdx]);
            idx = nextIdx;
            outputBuffer[n] += currentVal;
        }
    }

    for (float& s : outputBuffer) 
    {
        s = std::tanh(s / std::sqrt(static_cast<float>(chordFrequencies.size())));
    }

    //m_musicSystem->RenderAudio(outputBuffer);
    m_musicSystem->MixAudioAsync(outputBuffer);
}