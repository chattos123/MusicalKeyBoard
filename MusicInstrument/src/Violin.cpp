#include "Violin.h"
#include <cmath>
#include <random>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Violin::Violin(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)),
      m_name("Acoustic Violin"),
      m_openTuning{ 196.00, 293.66, 440.00, 659.25 }
{
}

std::string Violin::GetName() const {
    return m_name;
}

int Violin::GetStringCount() const {
    return static_cast<int>(m_openTuning.size());
}

void Violin::PlayNote(double frequencyHz, double durationSeconds, double velocity) {
    BowString(frequencyHz, durationSeconds, velocity);
}

void Violin::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) {
    if (frequencies.empty() || !m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    for (double f0 : frequencies) {
        for (size_t n = 0; n < totalSamples; ++n) {
            double t = static_cast<double>(n) / sampleRate;
            double attack = (t < 0.1) ? (t / 0.1) : (t > durationSeconds - 0.2 ? (durationSeconds - t) / 0.2 : 1.0);
            
            // Violin Helmholtz sawtooth-like harmonic series
            double sample = 0.0;
            for (int k = 1; k <= 8; ++k) {
                double f_k = k * f0;
                if (f_k >= sampleRate * 0.45) break;
                double amp = (1.0 / k) * (0.8 + 0.2 * (k % 2)); // Odd/even wooden body tilt
                sample += amp * std::sin(2.0 * M_PI * f_k * t);
            }
            outputBuffer[n] += static_cast<float>(sample * attack * 0.3);
        }
    }

    for (float& s : outputBuffer) {
        s = std::tanh(s / std::sqrt(static_cast<float>(frequencies.size())));
    }
    m_musicSystem->RenderAudio(outputBuffer);
}

void Violin::PluckString(int stringIndex, double durationSeconds, double velocity) {
    // Pizzicato using Karplus-Strong
    if (stringIndex < 0 || stringIndex >= static_cast<int>(m_openTuning.size()) || !m_musicSystem) return;

    double freq = m_openTuning[stringIndex];
    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    std::mt19937 rng(54321);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    size_t ringSize = static_cast<size_t>(sampleRate / freq);
    if (ringSize < 2) ringSize = 2;
    std::vector<float> ring(ringSize);
    for (size_t i = 0; i < ringSize; ++i) ring[i] = dist(rng);

    size_t idx = 0;
    const float decay = 0.985f; // Short pizzicato decay

    for (size_t n = 0; n < totalSamples; ++n) {
        float currentVal = ring[idx];
        size_t nextIdx = (idx + 1) % ringSize;
        ring[idx] = decay * 0.5f * (ring[idx] + ring[nextIdx]);
        idx = nextIdx;
        outputBuffer[n] = currentVal * static_cast<float>(velocity);
    }

    for (float& s : outputBuffer) s = std::tanh(s);
    m_musicSystem->RenderAudio(outputBuffer);
}

void Violin::Strum(const std::vector<double>& chordFrequencies, double strumTimeMs, double durationSeconds) {
    PlayChord(chordFrequencies, durationSeconds, 0.8);
}

void Violin::BowString(double frequencyHz, double durationSeconds, double bowPressure, double vibratoDepthHz, double vibratoRateHz) {
    if (!m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    double phase = 0.0;
    const int numHarmonics = 10;

    for (size_t n = 0; n < totalSamples; ++n) {
        double t = static_cast<double>(n) / sampleRate;

        // Smooth bow stroke envelope (fade-in attack and gentle release)
        double env = 1.0;
        if (t < 0.15) env = t / 0.15;
        else if (t > durationSeconds - 0.25) env = (durationSeconds - t) / 0.25;

        // Human vibrato begins ~0.2s after onset
        double currentVibrato = (t > 0.2) ? vibratoDepthHz * std::sin(2.0 * M_PI * vibratoRateHz * t) : 0.0;
        double currentFreq = frequencyHz + currentVibrato;

        // Rich body formants (simulating spruce top resonance)
        double sample = 0.0;
        for (int k = 1; k <= numHarmonics; ++k) {
            double fk = k * currentFreq;
            if (fk >= sampleRate * 0.45) break;

            // Formant peak around 2.5 kHz (bridge & soundboard resonance)
            double resonance = 1.0 / (1.0 + std::pow((fk - 2500.0) / 1000.0, 2.0));
            double amp = (1.0 / std::pow(k, 0.9)) * (0.6 + 0.4 * resonance);

            sample += amp * std::sin(k * phase);
        }

        phase += 2.0 * M_PI * currentFreq / sampleRate;
        if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;

        outputBuffer[n] = static_cast<float>(sample * env * bowPressure * 0.25);
    }

    for (float& s : outputBuffer) s = std::tanh(s);
    //m_musicSystem->RenderAudio(outputBuffer);
    m_musicSystem->MixAudioAsync(outputBuffer);
}