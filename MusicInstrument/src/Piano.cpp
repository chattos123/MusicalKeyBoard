#include "Piano.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Piano::Piano(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)), m_name("Grand Piano") {}

std::string Piano::GetName() const 
{
    return m_name;
}

void Piano::PlayNote(double frequencyHz, double durationSeconds, double velocity) 
{
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

void Piano::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) 
{
    if (frequencies.empty() || !m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    const double B = 0.00015; // Inharmonicity coefficient for piano strings
    const int numPartials = 6;

    for (double f0 : frequencies) {
        for (size_t n = 0; n < totalSamples; ++n) {
            double t = static_cast<double>(n) / sampleRate;
            double attack = (t < 0.005) ? (t / 0.005) : 1.0;
            double sample = 0.0;

            for (int k = 1; k <= numPartials; ++k) {
                double f_k = k * f0 * std::sqrt(1.0 + B * k * k);
                if (f_k >= sampleRate * 0.45) break; // Avoid Nyquist aliasing

                double fastDecay = (0.8 + 0.3 * k) * (f0 / 200.0);
                double slowDecay = (0.15 + 0.05 * k) * (f0 / 400.0);
                double amp = (1.0 / std::pow(k, 1.25)) * (0.6 + 0.4 * velocity);

                double env = 0.7 * std::exp(-fastDecay * t) + 0.3 * std::exp(-slowDecay * t);
                sample += amp * env * std::sin(2.0 * M_PI * f_k * t);
            }

            outputBuffer[n] += static_cast<float>(sample * attack * velocity);
        }
    }

    // Soft saturation & normalization
    for (float& s : outputBuffer) 
    {
        s = std::tanh(s / std::sqrt(static_cast<float>(frequencies.size())));
    }

    //m_musicSystem->RenderAudio(outputBuffer);
    m_musicSystem->MixAudioAsync(outputBuffer);
}