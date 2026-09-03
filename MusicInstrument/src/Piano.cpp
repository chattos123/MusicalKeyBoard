/**
 * @file Piano.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the Piano class modeling an acoustic grand piano.
 *
 * This file provides the concrete implementation of the Piano class, synthesizing struck-string
 * piano tones using an additive modal model. It captures key acoustic phenomena including stiff-string
 * inharmonicity (partial dispersion), multi-string unison double-decay envelopes, dynamic strike-velocity
 * timbre scaling, Nyquist anti-aliasing guards, and soundboard soft saturation.
 *
 * Design Choices:
 * - Uses additive synthesis across 6 partials to model individual string vibrational modes.
 * - Simulates physical wire stiffness using an inharmonicity dispersion formula:
 *   f_k = k * f_0 * sqrt(1 + B * k^2) with coefficient B = 0.00015.
 * - Implements a double exponential decay envelope (fast decay + slow reverberant decay) to replicate
 *   coupled multi-string unison physics and bridge energy transfer.
 * - Applies a short linear attack ramp (5 ms) to replicate hammer impact excitation without clicks.
 * - Normalizes polyphonic chords via square-root voice scaling and soft-saturates through std::tanh
 *   to emulate soundboard acoustic compression.
 * - Streams output PCM sample buffers asynchronously via IMusicSystem::MixAudioAsync.
 *
 * Physics & Acoustics Notes:
 * - Piano strings exhibit stiffness that introduces a restoring force in addition to tension,
 *   causing partials to sharp progressively as frequency increases (dispersive wave propagation).
 * - Multi-string unisons produce two distinct decay phases: an initial rapid decay from in-phase string
 *   motion exerting high bridge force, followed by a slower sustain as anti-phase motions decay gradually.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "Piano.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Constructs a Grand Piano instance bound to an audio subsystem.
 * @param musicSystem Shared pointer to the audio system backend for sound rendering.
 */
Piano::Piano(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)), m_name("Grand Piano") {}

/**
 * @brief Retrieves the human-readable identifier of the instrument.
 * @return Standard string containing the instrument name ("Grand Piano").
 */
std::string Piano::GetName() const 
{
    return m_name;
}

/**
 * @brief Synthesizes and plays a single piano note through the audio subsystem.
 * @param frequencyHz Fundamental pitch frequency in Hertz.
 * @param durationSeconds Sounding duration in seconds.
 * @param velocity Strike force scalar in the range [0.0, 1.0].
 */
void Piano::PlayNote(double frequencyHz, double durationSeconds, double velocity) 
{
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

/**
 * @brief Synthesizes and plays a polyphonic piano chord through the audio subsystem.
 * @details Evaluates an additive modal synthesis loop for each fundamental tone:
 *          - Calculates dispersed partial frequencies: f_k = k * f_0 * sqrt(1 + B * k^2).
 *          - Clamps frequencies below 0.45 * sampleRate to prevent Nyquist foldover.
 *          - Evaluates composite double-exponential decay envelopes per partial.
 *          - Applies dynamic velocity brightness scaling and a 5 ms hammer attack ramp.
 *          - Normalizes voice energy and soft-clips via std::tanh before dispatching to the mixer.
 * @param frequencies Vector of fundamental frequencies in Hertz composing the chord.
 * @param durationSeconds Sounding window duration in seconds.
 * @param velocity Strike force scalar in the range [0.0, 1.0].
 */
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