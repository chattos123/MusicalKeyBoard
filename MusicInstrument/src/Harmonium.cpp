#include "Harmonium.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Harmonium::Harmonium(std::shared_ptr<IMusicSystem> musicSystem, bool enableCoupler)
    : m_musicSystem(std::move(musicSystem)),
      m_name("Indian Hand-Pumped Harmonium"),
      m_couplerEnabled(enableCoupler)
{
}

std::string Harmonium::GetName() const {
    return m_name;
}

void Harmonium::SetCoupler(bool enabled) {
    m_couplerEnabled = enabled;
}

void Harmonium::PlayNote(double frequencyHz, double durationSeconds, double velocity) {
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

void Harmonium::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) {
    if (frequencies.empty() || !m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    const int numHarmonics = 12;

    for (double f0 : frequencies) {
        // Build reed frequency bank: Main reed + Octave coupled reed (if enabled) with subtle acoustic beating
        std::vector<std::pair<double, double>> reedBanks;
        reedBanks.push_back({ f0, 0.65 });             // Main Male reed
        reedBanks.push_back({ f0 + 0.35, 0.35 });      // Detuned chorus pair

        if (m_couplerEnabled && (f0 * 2.0 < sampleRate * 0.45)) {
            reedBanks.push_back({ f0 * 2.0, 0.45 });   // Female octave reed
            reedBanks.push_back({ f0 * 2.0 - 0.4, 0.25 }); // Upper octave chorus
        }

        for (size_t n = 0; n < totalSamples; ++n) {
            double t = static_cast<double>(n) / sampleRate;

            // Bellows envelope: fast swell, stable airflow sustain, gentle pressure drop
            double env = 1.0;
            if (t < 0.06) {
                env = t / 0.06; // Bellows air-intake swell
            } else if (t > durationSeconds - 0.12) {
                env = (durationSeconds - t) / 0.12; // Air release
            }

            // Subtle bellows pumping modulation (4.5 Hz air pulsation)
            double bellowsTremor = 1.0 + 0.04 * std::sin(2.0 * M_PI * 4.5 * t);

            double reedSample = 0.0;

            for (const auto& [reedFreq, reedWeight] : reedBanks) {
                for (int k = 1; k <= numHarmonics; ++k) {
                    double fk = k * reedFreq;
                    if (fk >= sampleRate * 0.45) break;

                    // Free-reed harmonic spectrum: prominent odd harmonics with steady high partial presence
                    double harmonicAmp = (1.0 / std::pow(k, 0.85)) * (k % 2 == 1 ? 1.0 : 0.65);

                    // Wooden body chamber resonance (formants around 800 Hz and 2.2 kHz)
                    double chamberFilter = 1.0 + 0.5 * std::exp(-std::pow((fk - 800.0) / 300.0, 2.0))
                                               + 0.3 * std::exp(-std::pow((fk - 2200.0) / 500.0, 2.0));

                    reedSample += reedWeight * harmonicAmp * chamberFilter * std::sin(2.0 * M_PI * fk * t);
                }
            }

            outputBuffer[n] += static_cast<float>(reedSample * env * bellowsTremor * velocity * 0.15);
        }
    }

    // Warm wooden box saturation
    for (float& s : outputBuffer) {
        s = std::tanh(s / std::sqrt(static_cast<float>(frequencies.size())));
    }

    m_musicSystem->MixAudioAsync(outputBuffer);
}

void Harmonium::PlayDrone(double rootFreqHz, double durationSeconds, double velocity) {
    // Sustained Sa-Pa (Root + Perfect Fifth) classic Indian classical drone
    double fifthFreq = rootFreqHz * 1.498307; // Just/Equal tempered fifth
    PlayChord({ rootFreqHz, fifthFreq }, durationSeconds, velocity);
}