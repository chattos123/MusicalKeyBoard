/**
 * @file BassGuitar.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the BassGuitar class for physical modeling audio synthesis.
 *
 * This file provides the concrete implementation of the BassGuitar class, simulating a
 * 4-string electric bass guitar using the Karplus-Strong digital waveguide synthesis algorithm.
 * It handles single-note plucking, polyphonic chord voicings, strumming delays, and low-frequency
 * noise filtering to emulate the acoustic dynamics of heavy wound strings.
 *
 * Design Choices:
 * - Implements Karplus-Strong string synthesis with low-pass filtered noise excitation to emulate
 *   heavy bass string gauge dampening.
 * - Employs a single-pole averaging feedback filter with a high decay coefficient (0.998) to model
 *   prolonged bass sustain.
 * - Staggered delay-line offsets simulate human strum sweeps across strings.
 * - Dynamic range compression uses hyperbolic tangent (std::tanh) soft-clipping to prevent digital
 *   overflow while introducing natural harmonic saturation.
 * - Direct dispatch via IMusicSystem::MixAudioAsync ensures non-blocking playback on the audio engine.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "BassGuitar.h"
#include <cmath>
#include <random>
#include <algorithm>

/**
 * @brief Constructs a BassGuitar instance with standard 4-string bass tuning.
 * @param musicSystem Shared pointer to the audio system backend for sound rendering.
 */
BassGuitar::BassGuitar(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)),
      m_name("4-String Electric Bass Guitar"),
      m_openTuning{ 41.20, 55.00, 73.42, 98.00 } // Standard tuning: E1, A1, D2, G2
{
}

/**
 * @brief Retrieves the human-readable identifier of the instrument.
 * @return Standard string containing the instrument name.
 */
std::string BassGuitar::GetName() const {
    return m_name;
}

/**
 * @brief Synthesizes and plays a single note through the audio subsystem.
 * @param frequencyHz Fundamental frequency of the target note in Hertz.
 * @param durationSeconds Duration of the sounding note in seconds (default: 1.5s).
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0] (default: 0.8).
 */
void BassGuitar::PlayNote(double frequencyHz, double durationSeconds, double velocity) {
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

/**
 * @brief Plays multiple simultaneous frequencies as an immediate chord.
 * @param frequencies Vector of fundamental frequencies (in Hertz) composing the chord.
 * @param durationSeconds Duration of the chord decay in seconds (default: 2.0s).
 * @param velocity Intensity scalar in the range [0.0, 1.0] (default: 0.8).
 */
void BassGuitar::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) {
    Strum(frequencies, 0.0, durationSeconds);
}

/**
 * @brief Retrieves the count of configured strings on the bass guitar.
 * @return Total number of strings (4).
 */
int BassGuitar::GetStringCount() const {
    return static_cast<int>(m_openTuning.size());
}

/**
 * @brief Plucks an open string by index according to standard bass tuning.
 * @param stringIndex Zero-based string index (0: E1, 1: A1, 2: D2, 3: G2).
 * @param durationSeconds Duration of the note ring in seconds (default: 1.5s).
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0] (default: 0.8).
 */
void BassGuitar::PluckString(int stringIndex, double durationSeconds, double velocity) {
    if (stringIndex >= 0 && stringIndex < static_cast<int>(m_openTuning.size())) {
        PlayNote(m_openTuning[stringIndex], durationSeconds, velocity);
    }
}

/**
 * @brief Strums a collection of frequencies using Karplus-Strong waveguide synthesis.
 * @details Computes per-string circular delay lines initialized with low-pass filtered noise
 *          (0.6 * curr + 0.4 * prev) to mimic the dark timbre of thick wound bass strings.
 *          Applies a two-point averaging loss filter per feedback cycle, sums voices with
 *          temporal roll offsets, normalizes amplitude with hyperbolic tangent soft clipping,
 *          and enqueues the resulting PCM stream asynchronously to the audio engine.
 * @param chordFrequencies Collection of fundamental frequencies in Hertz.
 * @param strumTimeMs Sweep offset between successive string strikes in milliseconds.
 * @param durationSeconds Overall decay and ring time in seconds.
 */
void BassGuitar::Strum(const std::vector<double>& chordFrequencies, double strumTimeMs, double durationSeconds) {
    if (chordFrequencies.empty() || !m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    std::mt19937 rng(4242);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (size_t i = 0; i < chordFrequencies.size(); ++i) {
        double freq = chordFrequencies[i];
        size_t delayOffset = static_cast<size_t>((i * strumTimeMs / 1000.0) * sampleRate);
        size_t ringSize = static_cast<size_t>(sampleRate / freq);
        if (ringSize < 2) ringSize = 2;

        std::vector<float> ring(ringSize);
        float prevNoise = 0.0f;
        for (size_t k = 0; k < ringSize; ++k) {
            float raw = dist(rng);
            ring[k] = 0.6f * raw + 0.4f * prevNoise;
            prevNoise = raw;
        }

        size_t idx = 0;
        const float decay = 0.998f;

        for (size_t n = delayOffset; n < totalSamples; ++n) {
            float currentVal = ring[idx];
            size_t nextIdx = (idx + 1) % ringSize;

            ring[idx] = decay * 0.5f * (ring[idx] + ring[nextIdx]);
            idx = nextIdx;

            outputBuffer[n] += currentVal;
        }
    }

    for (float& s : outputBuffer) {
        s = std::tanh(s * 1.2f / std::sqrt(static_cast<float>(chordFrequencies.size())));
    }

    m_musicSystem->MixAudioAsync(outputBuffer);
}