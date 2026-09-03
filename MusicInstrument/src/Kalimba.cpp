/**
 * @file Kalimba.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the Kalimba class for lamellophone physical modeling synthesis.
 *
 * This file contains the implementation of the Kalimba class, modeling a 17-key African thumb
 * piano (lamellophone). The sound generation simulates metallic tine vibrations clamped at one end,
 * producing characteristic inharmonic overtones, fast transient plucks, and prolonged fundamental resonance.
 *
 * Design Choices:
 * - Employs physical modal additive synthesis combining a fundamental sine tone and an inharmonic tine partial.
 * - Uses a steep linear attack envelope (2 ms) to replicate the hard transient of a fingernail or thumb pluck.
 * - Decouples decay rates: the fundamental exhibits prolonged resonance (decay rate 2.5/s), while the inharmonic
 *   tine partial decays rapidly (decay rate 12.0/s).
 * - Multi-voice polyphony is scaled dynamically using square-root voice normalization and soft-clipped via std::tanh.
 * - Dispatches output PCM buffers asynchronously to IMusicSystem::MixAudioAsync for non-blocking playback.
 *
 * Physics & Acoustics Notes:
 * - Unlike idealized harmonic strings or open acoustic tubes, a clamped cantilever beam (metal tine) exhibits
 *   stiff, non-integer modal overtones. The first prominent transversal flexural mode occurs at approximately
 *   5.4 times the fundamental frequency: f_1 ≈ 5.4 * f_0.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "Kalimba.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Constructs a 17-Key Kalimba instance bound to an audio subsystem.
 * @param musicSystem Shared pointer to the audio system backend for sound rendering.
 */
Kalimba::Kalimba(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)), m_name("17-Key Kalimba") 
{

}

/**
 * @brief Retrieves the human-readable identifier of the instrument.
 * @return Standard string containing the instrument name.
 */
std::string Kalimba::GetName() const 
{
    return m_name;
}

/**
 * @brief Synthesizes and plays a single kalimba note through the audio subsystem.
 * @param frequencyHz Fundamental frequency of the note in Hertz.
 * @param durationSeconds Duration of the sounding note in seconds.
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0].
 */
void Kalimba::PlayNote(double frequencyHz, double durationSeconds, double velocity) 
{
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

/**
 * @brief Synthesizes and plays a polyphonic kalimba chord through the audio subsystem.
 * @details For each specified pitch, evaluates a clamped-tine cantilever modal model:
 *          - Rapid linear pluck attack (2 ms).
 *          - Prolonged fundamental resonance decay: exp(-2.5 * t).
 *          - Sharp inharmonic overtone partial (5.4 * f0) with fast damping: exp(-12.0 * t).
 *          Applies square-root voice scaling and hyperbolic tangent (std::tanh) soft limiting
 *          across the composite buffer before streaming to the hardware mixer.
 * @param frequencies Vector of fundamental frequencies in Hertz composing the chord.
 * @param durationSeconds Total sound duration in seconds.
 * @param velocity Pluck intensity scalar in the range [0.0, 1.0].
 */
void Kalimba::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) 
{
    if (frequencies.empty() || !m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    for (double f0 : frequencies) 
    {
        for (size_t n = 0; n < totalSamples; ++n) 
        {
            double t = static_cast<double>(n) / sampleRate;
            double attack = (t < 0.002) ? (t / 0.002) : 1.0;
            double decayFund = std::exp(-2.5 * t);
            double decayTine = std::exp(-12.0 * t);

            double fund = std::sin(2.0 * M_PI * f0 * t) * decayFund;
            double overtone = 0.4 * std::sin(2.0 * M_PI * f0 * 5.4 * t) * decayTine;

            outputBuffer[n] += static_cast<float>((fund + overtone) * attack * velocity);
        }
    }

    for (float& s : outputBuffer) 
    {
        s = std::tanh(s / std::sqrt(static_cast<float>(frequencies.size())));
    }

    //m_musicSystem->RenderAudio(outputBuffer);
    m_musicSystem->MixAudioAsync(outputBuffer);
}