#include "Trumpet.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Trumpet::Trumpet(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)),
      m_name("Bb Brass Trumpet"),
      m_mute(BrassMuteType::None)
{
}

std::string Trumpet::GetName() const {
    return m_name;
}

void Trumpet::SetMute(BrassMuteType mute) {
    m_mute = mute;
}

void Trumpet::PlayNote(double frequencyHz, double durationSeconds, double velocity) 
{
    if (!m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    double phase = 0.0;
    const int numHarmonics = 12;

    for (size_t n = 0; n < totalSamples; ++n) {
        double t = static_cast<double>(n) / sampleRate;

        // Brass lip-reed envelope: fast burst attack, stable sustain, sharp cutoff
        double env = 1.0;
        if (t < 0.04) env = t / 0.04;
        else if (t > durationSeconds - 0.08) env = (durationSeconds - t) / 0.08;

        // Dynamic spectral brightness (harder blowing produces much brighter high harmonics)
        double brightness = 1.0 + 1.5 * velocity;

        double sample = 0.0;
        for (int k = 1; k <= numHarmonics; ++k) {
            double fk = k * frequencyHz;
            if (fk >= sampleRate * 0.45) break;

            double harmonicWeight = std::pow(static_cast<double>(k), -1.2 / brightness);

            // Mute filter adjustments
            if (m_mute == BrassMuteType::Harmon) {
                // High buzzing resonant peak
                harmonicWeight *= (k >= 3 && k <= 6) ? 2.2 : 0.4;
            } else if (m_mute == BrassMuteType::Straight) {
                // High pass emphasis
                harmonicWeight *= (k >= 4) ? 1.5 : 0.6;
            }

            sample += harmonicWeight * std::sin(k * phase);
        }

        phase += 2.0 * M_PI * frequencyHz / sampleRate;
        if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;

        outputBuffer[n] = static_cast<float>(sample * env * velocity * 0.28);
    }

    // Brass bell non-linear wave shaping
    for (float& s : outputBuffer) {
        s = std::tanh(s * 1.3f);
    }

    //m_musicSystem->RenderAudio(outputBuffer);
    m_musicSystem->MixAudioAsync(outputBuffer);
}

void Trumpet::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) {
    for (double f : frequencies) {
        PlayNote(f, durationSeconds, velocity);
    }
}

void Trumpet::Tonguing(double frequencyHz, int noteCount, double noteDuration, double velocity) {
    for (int i = 0; i < noteCount; ++i) {
        PlayNote(frequencyHz, noteDuration, velocity);
    }
}

void Trumpet::PlayFanfare(const std::vector<double>& notes, double tempoBpm) {
    double beatSec = 60.0 / tempoBpm;
    for (double note : notes) {
        PlayNote(note, beatSec * 0.75, 0.9);
    }
}