/**
 * @file Saxophone.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the Saxophone class for single-reed conical aerophone synthesis.
 *
 * This file contains the implementation of the Saxophone class, modeling an alto saxophone
 * using additive harmonic synthesis and dynamic breath envelopes. It provides single-note
 * playback, polyphonic chord voicings, acoustic mute configuration, and staccato tonguing
 * re-articulation.
 *
 * Design Choices:
 * - Uses additive synthesis across 4 harmonic partials (both even and odd) to capture the
 *   characteristic harmonic richness of conical-bore woodwinds.
 * - Applies a linear trapezoidal breath envelope (50 ms attack swell, 80 ms release decay)
 *   to emulate player lung pressure and prevent discontinuous boundary clicks.
 * - Polyphonic chords are rendered by dishing out attenuated component notes across voices.
 * - Tonguing simulates rapid reed-damping re-articulations separated by a thread sleep interval.
 * - Streams output PCM sample buffers asynchronously via IMusicSystem::MixAudioAsync.
 *
 * Physics & Acoustics Notes:
 * - Because the saxophone has a conical bore, it operates acoustically as an open pipe,
 *   supporting all integer harmonics (1f, 2f, 3f, 4f, ...) unlike cylindrical single-reed
 *   instruments like the clarinet, which predominantly generate odd harmonics.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "Saxophone.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

/**
 * @brief Constructs a Saxophone instance bound to an audio subsystem.
 * @param system Shared pointer to the audio system backend for sound rendering.
 */
Saxophone::Saxophone(std::shared_ptr<IMusicSystem> system)
    : m_system(std::move(system)) 
{
}

/**
 * @brief Retrieves the human-readable identifier of the instrument.
 * @return Standard string containing the instrument name ("Saxophone").
 */
std::string Saxophone::GetName() const 
{
    return "Saxophone";
}

/**
 * @brief Configures the active acoustic mute type for the instrument.
 * @param mute The BrassMuteType modifier to apply.
 */
void Saxophone::SetMute(BrassMuteType mute) 
{
    m_currentMute = mute;
}

/**
 * @brief Synthesizes and plays a polyphonic wind voicing through the audio hardware.
 * @param frequencies Vector of fundamental frequencies in Hertz composing the chord.
 * @param duration Duration of the chord in seconds.
 * @param velocity Intensity scalar in the range [0.0, 1.0].
 */
void Saxophone::PlayChord(const std::vector<double>& frequencies, double duration, double velocity) 
{
    for (double f : frequencies) 
    {
        PlayNote(f, duration, velocity * 0.7);
    }
}

/**
 * @brief Executes rapid staccato tonguing pulses on a note.
 * @details Sequentially triggers staccato note bursts with a 15% silence gap between successive
 *          articulations, pausing the calling thread by noteDuration to maintain rhythmic timing.
 * @param frequencyHz Fundamental sounding frequency in Hertz.
 * @param noteCount Total number of staccato pulses in the articulation sequence.
 * @param noteDuration Duration of each individual note pulse in seconds.
 * @param velocity Breath strike intensity scalar in the range [0.0, 1.0].
 */
void Saxophone::Tonguing(double frequencyHz, int noteCount, double noteDuration, double velocity) 
{
    for (int i = 0; i < noteCount; ++i) 
    {
        PlayNote(frequencyHz, noteDuration * 0.85, velocity);
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(noteDuration * 1000.0)));
    }
}

/**
 * @brief Synthesizes and plays a single sustained saxophone note through the audio subsystem.
 * @details Evaluates additive conical-pipe harmonic synthesis with four partials, shapes the
 *          waveform using a 50 ms attack and 80 ms release trapezoidal amplitude envelope, clamps
 *          samples to [-1.0, 1.0], and dispatches the buffer to the asynchronous audio mixer.
 * @param frequency Fundamental pitch frequency in Hertz.
 * @param duration Total sounding duration in seconds.
 * @param velocity Normalized breath pressure scalar in the range [0.0, 1.0].
 */
void Saxophone::PlayNote(double frequency, double duration, double velocity) 
{
    if (!m_system || frequency <= 0.0 || duration <= 0.0) return;

    double sampleRate = m_system->GetSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;

    size_t totalSamples = static_cast<size_t>(sampleRate * duration);
    std::vector<float> buffer(totalSamples, 0.0f);

    const double twoPi = 6.283185307179586;
    double phase = 0.0;
    double phaseInc = (twoPi * frequency) / sampleRate;

    for (size_t i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;

        // Linear envelope fade in/out to guarantee audible sound without clicks
        double env = 1.0;
        if (t < 0.05) 
        {
            env = t / 0.05;
        } 
        else if (t > (duration - 0.08)) 
        {
            env = (duration - t) / 0.08;
        }

        phase += phaseInc;
        if (phase >= twoPi) phase -= twoPi;

        // Rich reed harmonics: 1st, 2nd, 3rd, 4th, 5th
        double s = std::sin(phase)
                 + 0.50 * std::sin(2.0 * phase)
                 + 0.35 * std::sin(3.0 * phase)
                 + 0.15 * std::sin(4.0 * phase);

        buffer[i] = static_cast<float>(std::clamp(s * env * velocity * 0.5, -1.0, 1.0));
    }

    m_system->MixAudioAsync(buffer);
}