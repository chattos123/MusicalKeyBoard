/**
 * @file Guitar.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the Guitar class for acoustic steel-string guitar physical modeling.
 *
 * This file contains the implementation of the Guitar class, simulating a 6-string acoustic
 * steel-string guitar via the Karplus-Strong digital waveguide synthesis algorithm. It provides
 * single-note plucking, polyphonic chord voicing, and humanized strumming simulation with
 * temporal displacement between successive string excitations.
 *
 * Design Choices:
 * - Implements Karplus-Strong waveguide synthesis using dynamic ring buffers initialized with
 *   uniform white noise [-1.0, 1.0] to model string plucks.
 * - Employs a single-pole two-point averaging loss filter (decay factor: 0.996) to simulate natural
 *   high-frequency damping and acoustic body resonance decay.
 * - Simulates human strum dynamics by offsetting each string's onset using strumTimeMs.
 * - Uses hyperbolic tangent (std::tanh) soft-clipping normalized across voice counts to ensure
 *   transparent dynamic headroom without hard clipping.
 * - Enqueues rendered PCM audio buffers asynchronously to IMusicSystem::MixAudioAsync for low-latency playback.
 *
 * Physics & Acoustics Notes:
 * - Standard tuning fundamental frequencies: E2 (82.41 Hz), A2 (110.00 Hz), D3 (146.83 Hz),
 *   G3 (196.00 Hz), B3 (246.94 Hz), E4 (329.63 Hz).
 * - Ring buffer size is determined by the fundamental period: N = sampleRate / frequencyHz.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "Guitar.h"
#include <cmath>
#include <random>
#include <algorithm>

/**
 * @brief Constructs an Acoustic Steel-String Guitar instance with standard 6-string tuning.
 * @param musicSystem Shared pointer to the audio system backend for sound rendering.
 */
Guitar::Guitar(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)), m_name("Acoustic Steel-String Guitar"),
      m_openTuning{ 82.41, 110.00, 146.83, 196.00, 246.94, 329.63 } {}

/**
 * @brief Retrieves the human-readable identifier of the instrument.
 * @return Standard string containing the instrument name.
 */
std::string Guitar::GetName() const 
{
    return m_name;
}

/**
 * @brief Synthesizes and plays a single guitar note through the audio subsystem.
 * @param frequencyHz Fundamental frequency of the note in Hertz.
 * @param durationSeconds Sound duration in seconds.
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0].
 */
void Guitar::PlayNote(double frequencyHz, double durationSeconds, double velocity) 
{
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

/**
 * @brief Plays multiple simultaneous frequencies as an immediate chord.
 * @param frequencies Vector of fundamental frequencies in Hertz composing the chord.
 * @param durationSeconds Duration of the chord decay in seconds.
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0].
 */
void Guitar::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) 
{
    Strum(frequencies, 0.0, durationSeconds);
}

/**
 * @brief Retrieves the count of configured physical strings on the guitar.
 * @return Total number of open strings (6).
 */
int Guitar::GetStringCount() const 
{
    return static_cast<int>(m_openTuning.size());
}

/**
 * @brief Plucks an open string by index based on standard tuning.
 * @param stringIndex Zero-based string index (0: E2, 1: A2, 2: D3, 3: G3, 4: B3, 5: E4).
 * @param durationSeconds Total ringing duration in seconds.
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0].
 */
void Guitar::PluckString(int stringIndex, double durationSeconds, double velocity) 
{
    if (stringIndex >= 0 && stringIndex < static_cast<int>(m_openTuning.size())) 
    {
        PlayNote(m_openTuning[stringIndex], durationSeconds, velocity);
    }
}

/**
 * @brief Strums a collection of frequencies using the Karplus-Strong algorithm with staggered offsets.
 * @details Allocates circular ring buffers for each string based on fundamental frequency (sampleRate / freq).
 *          Initializes delay lines with white noise bursts, simulates string vibration decay using a
 *          two-point averaging low-pass feedback loop (decay = 0.996), accumulates samples into a shared
 *          output buffer with per-string time delays (strumTimeMs), applies a soft saturation tanh curve,
 *          and enqueues the audio into the mixer pipeline.
 * @param chordFrequencies Collection of fundamental frequencies in Hertz representing the chord.
 * @param strumTimeMs Time delay in milliseconds between consecutive string strikes.
 * @param durationSeconds Total sounding and decay duration in seconds.
 */
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