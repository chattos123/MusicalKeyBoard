/**
 * @file Mandolin.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the Mandolin class for dual-course plucked string synthesis.
 *
 * This file contains the concrete implementation of the Mandolin class, simulating an
 * 8-string (4-course) acoustic mandolin using the Karplus-Strong digital waveguide algorithm.
 * It models paired-string unison chorus detuning, high-register string decay dynamics,
 * humanized strum sweeps, and rapid tremolo picking articulation.
 *
 * Design Choices:
 * - Simulates physical 2-string courses by instantiating dual Karplus-Strong delay lines per note,
 *   detuned by +/-0.35 Hz to produce natural acoustic chorusing and phase beating.
 * - Uses a high-register damping coefficient (decay = 0.991 for open plucks, 0.988 for tremolo strikes)
 *   reflecting the short sustain of small-body acoustic instruments under high string tension.
 * - Simulates tremolo picking by scheduling repeated re-excitations at a selectable stroke rate (rateHz),
 *   allowing overlapping reverberant tails to produce continuous sustain.
 * - Dynamically normalizes multi-voice energy using square-root course scaling and soft-saturates
 *   via std::tanh to emulate the acoustic resonance of a carved spruce top.
 * - Directs strum audio to IMusicSystem::MixAudioAsync and rendered tremolo buffers to IMusicSystem::RenderAudio.
 *
 * Physics & Acoustics Notes:
 * - Standard tuning fundamental frequencies across the 4 paired courses:
 *   Course 0 (G3): 196.00 Hz, Course 1 (D4): 293.66 Hz, Course 2 (A4): 440.00 Hz, Course 3 (E5): 659.25 Hz.
 * - Total string count equals 8 across 4 distinct paired courses.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "Mandolin.h"
#include <cmath>
#include <random>
#include <algorithm>

/**
 * @brief Constructs an 8-String Acoustic Mandolin instance bound to an audio subsystem.
 * @param musicSystem Shared pointer to the audio system backend for sound rendering.
 */
Mandolin::Mandolin(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)),
      m_name("8-String Acoustic Mandolin"),
      m_courseTunings{ 196.00, 293.66, 440.00, 659.25 } // G3, D4, A4, E5
{
}

/**
 * @brief Retrieves the human-readable identifier of the instrument.
 * @return Standard string containing the instrument name.
 */
std::string Mandolin::GetName() const {
    return m_name;
}

/**
 * @brief Retrieves the total count of physical strings on the instrument.
 * @return Total number of strings (8 strings configured as 4 paired courses).
 */
int Mandolin::GetStringCount() const {
    return 8; // 4 paired courses
}

/**
 * @brief Synthesizes and plays a single note through the audio subsystem.
 * @param frequencyHz Fundamental frequency of the note in Hertz.
 * @param durationSeconds Sounding duration in seconds.
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0].
 */
void Mandolin::PlayNote(double frequencyHz, double durationSeconds, double velocity) {
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

/**
 * @brief Plays multiple simultaneous frequencies as an immediate chord.
 * @param frequencies Vector of fundamental frequencies in Hertz composing the chord.
 * @param durationSeconds Sounding duration in seconds.
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0].
 */
void Mandolin::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) {
    Strum(frequencies, 0.0, durationSeconds);
}

/**
 * @brief Plucks a specific course of paired strings according to open tuning.
 * @param courseIndex Zero-based course index (0: G3, 1: D4, 2: A4, 3: E5).
 * @param durationSeconds Ring duration in seconds.
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0].
 */
void Mandolin::PluckString(int courseIndex, double durationSeconds, double velocity) 
{
    if (courseIndex >= 0 && courseIndex < static_cast<int>(m_courseTunings.size())) 
    {
        PlayNote(m_courseTunings[courseIndex], durationSeconds, velocity);
    }
}

/**
 * @brief Strums a collection of frequencies using dual-string Karplus-Strong waveguide synthesis.
 * @details For each chord pitch, instantiates a pair of micro-detuned delay lines (baseFreq +/- 0.35 Hz)
 *          to recreate the signature chorused timbre of paired mandolin courses. Applies a two-point
 *          averaging loss filter (decay = 0.991) per feedback cycle, sums voices with temporal roll
 *          offsets (strumTimeMs), soft-clips the result with std::tanh, and routes the buffer to
 *          the asynchronous audio mixer.
 * @param chordFrequencies Collection of fundamental frequencies in Hertz representing the chord.
 * @param strumTimeMs Sweep offset between adjacent course strikes in milliseconds.
 * @param durationSeconds Total sound duration in seconds.
 */
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

/**
 * @brief Executes a rapid tremolo picking sequence on a single note through the audio subsystem.
 * @details Re-excites paired micro-detuned strings at regular intervals determined by rateHz,
 *          allowing each stroke's ring buffer to decay (decay = 0.988) into subsequent strokes.
 *          Applies velocity scaling, soft-clips via std::tanh, and streams the output directly
 *          using IMusicSystem::RenderAudio.
 * @param frequencyHz Fundamental pitch frequency in Hertz.
 * @param durationSeconds Total duration of the tremolo passage in seconds.
 * @param rateHz Tremolo picking rate in strokes per second.
 * @param velocity Stroke force intensity scalar in the range [0.0, 1.0].
 */
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