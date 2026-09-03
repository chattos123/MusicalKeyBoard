/**
 * @file Harmonium.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the Harmonium class modeling an Indian hand-pumped free-reed harmonium.
 *
 * This file provides the concrete implementation of the Harmonium class, synthesizing free-reed
 * aero-acoustic dynamics, dual-reed acoustic chorus beating, mechanical octave coupler linkages,
 * continuous bellows pumping tremor, wooden resonance chamber formants, and classical Indian
 * drone voicings (Sa-Pa).
 *
 * Design Choices:
 * - Employs additive harmonic synthesis across 12 partials with free-reed spectral characteristics
 *   (accentuated odd partials decaying with power law k^0.85).
 * - Simulates physical paired reeds (Male and Female sets) with slight micro-detuning (0.35 Hz to 0.4 Hz)
 *   to produce authentic acoustic chorusing and beating.
 * - Simulates hand bellows airflow through a composite envelope: linear attack air swell (60 ms),
 *   gentle pressure decay release (120 ms), and periodic low-frequency pumping tremor (4.5 Hz sine modulation).
 * - Emulates acoustic wooden cavity filtration using dual Gaussian formants centered at 800 Hz and 2.2 kHz.
 * - Applies soft-saturation dynamic compression via std::tanh to emulate the warm acoustic saturation
 *   of the wooden sound chamber.
 * - Enqueues rendered PCM audio buffers asynchronously to IMusicSystem::MixAudioAsync for low-latency playback.
 *
 * Physics & Acoustics Notes:
 * - Free-reed oscillation operates via pressure differentials causing a brass tongue to slice through
 *   a close-tolerance frame, generating rich harmonic spectra with both odd and even partials.
 * - The octave coupler mechanically couples a key strike to its corresponding octave reed above,
 *   doubling the sounding density and upper-register shimmer.
 * - Indian classical drones typically articulate the tonic (Sa) combined with the fifth (Pa) at a
 *   frequency ratio of ~1.498307, generating a continuous harmonic bed.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "Harmonium.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Constructs an Indian Hand-Pumped Harmonium instance bound to an audio subsystem.
 * @param musicSystem Shared pointer to the audio system backend for sound rendering.
 * @param enableCoupler Initial state of the mechanical octave coupler mechanism (default: false).
 */
Harmonium::Harmonium(std::shared_ptr<IMusicSystem> musicSystem, bool enableCoupler)
    : m_musicSystem(std::move(musicSystem)),
      m_name("Indian Hand-Pumped Harmonium"),
      m_couplerEnabled(enableCoupler)
{
}

/**
 * @brief Retrieves the human-readable identifier of the harmonium.
 * @return Standard string containing the instrument name.
 */
std::string Harmonium::GetName() const {
    return m_name;
}

/**
 * @brief Enables or disables the mechanical octave coupler mechanism.
 * @param enabled True to engage octave reed coupling; false for standard dual-reed voicing.
 */
void Harmonium::SetCoupler(bool enabled) {
    m_couplerEnabled = enabled;
}

/**
 * @brief Synthesizes and plays a single harmonium note through the audio subsystem.
 * @param frequencyHz Fundamental frequency of the note in Hertz.
 * @param durationSeconds Total sound duration in seconds.
 * @param velocity Airflow intensity scalar in the range [0.0, 1.0].
 */
void Harmonium::PlayNote(double frequencyHz, double durationSeconds, double velocity) {
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

/**
 * @brief Synthesizes and plays a polyphonic harmonium chord through the audio subsystem.
 * @details Synthesizes dual-reed sets (main male reed and detuned chorus partner) along with
 *          octave-coupled reeds when enabled. Evaluates a 12-harmonic additive synthesis model
 *          modulated by an air-swell bellows envelope, 4.5 Hz hand-pumping tremor, dual-band
 *          wooden body chamber formants (800 Hz and 2200 Hz), and dynamic soft-saturation
 *          clipping before routing PCM audio to the hardware mixer.
 * @param frequencies Vector of fundamental frequencies in Hertz composing the chord.
 * @param durationSeconds Total chord sounding duration in seconds.
 * @param velocity Airflow intensity scalar in the range [0.0, 1.0].
 */
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

/**
 * @brief Plays a sustained Indian classical drone voicing (Sa-Pa).
 * @details Sounds the fundamental root frequency (Sa) paired with the fifth harmonic interval
 *          (Pa at frequency ratio ~1.498307) simultaneously via PlayChord to provide a constant
 *          harmonic foundation.
 * @param rootFreqHz Fundamental tonic frequency (Sa) in Hertz.
 * @param durationSeconds Total sounding duration of the drone in seconds.
 * @param velocity Airflow intensity scalar in the range [0.0, 1.0].
 */
void Harmonium::PlayDrone(double rootFreqHz, double durationSeconds, double velocity) {
    // Sustained Sa-Pa (Root + Perfect Fifth) classic Indian classical drone
    double fifthFreq = rootFreqHz * 1.498307; // Just/Equal tempered fifth
    PlayChord({ rootFreqHz, fifthFreq }, durationSeconds, velocity);
}