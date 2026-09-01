#include "Mandolin.h"
#include <cmath>
#include <random>
#include <algorithm>

Mandolin::Mandolin(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)),
      m_name("8-String Acoustic Mandolin"),
      m_courseTunings{ 196.00, 293.66, 440.00, 659.25 } // G3, D4, A4, E5
{
}

std::string Mandolin::GetName() const {
    return m_name;
}

int Mandolin::GetStringCount() const {
    return 8; // 4 paired courses
}

void Mandolin::PlayNote(double frequencyHz, double durationSeconds, double velocity) {
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

void Mandolin::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) {
    Strum(frequencies, 0.0, durationSeconds);
}

void Mandolin::PluckString(int courseIndex, double durationSeconds, double velocity) 
{
    if (courseIndex >= 0 && courseIndex < static_cast<int>(m_courseTunings.size())) 
    {
        PlayNote(m_courseTunings[courseIndex], durationSeconds, velocity);
    }
}

void Mandolin::Strum(const std::vector<double>& chordFrequencies, double strumTimeMs, double durationSeconds) 
{
    if (chordFrequencies.empty() || !m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    std::mt19937 rng(999);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (size_t chordIdx = 0; chordIdx < chordFrequencies.size(); ++chordIdx) 
    {
        double baseFreq = chordFrequencies[chordIdx];
        size_t delayOffset = static_cast<size_t>((chordIdx * strumTimeMs / 1000.0) * sampleRate);

        // Mandolin double-string unison chorus: 2 strings per course with slight micro-detuning
        double coursePairFreqs[2] = { baseFreq - 0.35, baseFreq + 0.35 };

        for (int pair = 0; pair < 2; ++pair) {
            double freq = coursePairFreqs[pair];
            size_t ringSize = static_cast<size_t>(sampleRate / freq);
            if (ringSize < 2) ringSize = 2;

            std::vector<float> ring(ringSize);
            for (size_t k = 0; k < ringSize; ++k) ring[k] = dist(rng);

            size_t idx = 0;
            const float decay = 0.991f; // Quick high-register pluck decay

            for (size_t n = delayOffset; n < totalSamples; ++n) 
            {
                float currentVal = ring[idx];
                size_t nextIdx = (idx + 1) % ringSize;

                // Moving average filter
                ring[idx] = decay * 0.5f * (ring[idx] + ring[nextIdx]);
                idx = nextIdx;

                outputBuffer[n] += currentVal;
            }
        }
    }

    // Soft saturation
    for (float& s : outputBuffer) {
        s = std::tanh(s * 0.8f / std::sqrt(static_cast<float>(chordFrequencies.size() * 2)));
    }

    //m_musicSystem->RenderAudio(outputBuffer);
    m_musicSystem->MixAudioAsync(outputBuffer);
}

void Mandolin::PlayTremolo(double frequencyHz, double durationSeconds, double rateHz, double velocity) 
{
    if (!m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    std::mt19937 rng(777);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    size_t strokeIntervalFrames = static_cast<size_t>(sampleRate / rateHz);
    size_t currentStrokeFrame = 0;

    while (currentStrokeFrame < totalSamples) {
        double pairFreqs[2] = { frequencyHz - 0.35, frequencyHz + 0.35 };

        for (int pair = 0; pair < 2; ++pair) {
            size_t ringSize = static_cast<size_t>(sampleRate / pairFreqs[pair]);
            if (ringSize < 2) ringSize = 2;

            std::vector<float> ring(ringSize);
            for (size_t k = 0; k < ringSize; ++k) ring[k] = dist(rng);

            size_t idx = 0;
            const float decay = 0.988f;

            for (size_t n = currentStrokeFrame; n < totalSamples; ++n) {
                float currentVal = ring[idx];
                size_t nextIdx = (idx + 1) % ringSize;

                ring[idx] = decay * 0.5f * (ring[idx] + ring[nextIdx]);
                idx = nextIdx;

                outputBuffer[n] += currentVal * static_cast<float>(velocity);
            }
        }

        currentStrokeFrame += strokeIntervalFrames;
    }

    for (float& s : outputBuffer) {
        s = std::tanh(s * 0.7f);
    }

    m_musicSystem->RenderAudio(outputBuffer);
}