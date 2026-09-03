/**
 * @file Trumpet.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the Trumpet class for lip-reed brass physical modeling synthesis.
 *
 * This file contains the implementation of the Trumpet class, simulating an acoustic Bb brass
 * trumpet using additive harmonic synthesis. It models non-linear lip-reed driving dynamics,
 * velocity-dependent spectral brightness (harmonic expansion), acoustic mute filtering
 * (Straight and Harmon), hyperbolic tangent bell radiation saturation, staccato tonguing,
 * and fanfare phrasing.
 *
 * Design Choices:
 * - Uses additive synthesis across up to 12 harmonics to capture bright brass partials.
 * - Simulates dynamic spectral brightness where increased blowing pressure/velocity flattens
 *   harmonic roll-off, yielding a brilliant, cutting timbre.
 * - Implements discrete spectral filtering for BrassMuteType (Harmon resonant bandpass boost,
 *   Straight high-pass emphasis).
 * - Applies a trapezoidal breath envelope (40 ms attack, 80 ms release) to eliminate start/end clicks.
 * - Uses hyperbolic tangent (std::tanh) soft saturation to emulate non-linear sound propagation
 *   and shock-wave steepening at the brass bell flare.
 * - Streams output PCM buffers asynchronously via IMusicSystem::MixAudioAsync.
 *
 * Physics & Acoustics Notes:
 * - Brass lip-reed mechanics cause higher harmonics to generate exponentially with increased
 *   dynamic level (acoustic pressure).
 * - Straight mutes attenuate low frequencies and enhance upper-register presence.
 * - Harmon mutes introduce a resonant acoustic cavity that notches the lower spectrum while
 *   strongly accentuating the 3rd through 6th harmonics for a tight, buzzy timbre.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "Trumpet.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Constructs a Bb Brass Trumpet instance bound to an audio subsystem.
 * @param musicSystem Shared pointer to the audio system backend for sound rendering.
 */
Trumpet::Trumpet(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)),
      m_name("Bb Brass Trumpet"),
      m_mute(BrassMuteType::None)
{
}

/**
 * @brief Retrieves the human-readable identifier of the instrument.
 * @return Standard string containing the instrument name ("Bb Brass Trumpet").
 */
std::string Trumpet::GetName() const 
{
    return m_name;
}

/**
 * @brief Configures the active acoustic mute modifier on the trumpet.
 * @param mute The BrassMuteType modifier to apply (None, Straight, Harmon).
 */
void Trumpet::SetMute(BrassMuteType mute) 
{
    m_mute = mute;
}

/**
 * @brief Synthesizes and plays a single brass note through the audio subsystem.
 * @details Evaluates a 12-harmonic additive model with dynamic spectral brightness scaling:
 *          - Applies breath envelope (40 ms attack, 80 ms decay).
 *          - Scales harmonic weight: k^(-1.2 / brightness), where brightness = 1.0 + 1.5 * velocity.
 *          - Applies Harmon or Straight mute filtering modifications across partials.
 *          - Clamps frequencies below Nyquist (0.45 * sampleRate).
 *          - Waveshapes output with std::tanh(s * 1.3f) to simulate flared bell radiation.
 * @param frequencyHz Fundamental frequency of the note in Hertz.
 * @param durationSeconds Sounding duration in seconds.
 * @param velocity Blowing pressure and attack intensity scalar in the range [0.0, 1.0].
 */
void Trumpet::PlayNote(double frequencyHz, double durationSeconds, double velocity) 
{
    if (!m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    double phase = 0.0;
    const int numHarmonics = 12;

    for (size_t n = 0; n < totalSamples; ++n) 
    {
        double t = static_cast<double>(n) / sampleRate;

        // Brass lip-reed envelope: fast burst attack, stable sustain, sharp cutoff
        double env = 1.0;
        if (t < 0.04) env = t / 0.04;
        else if (t > durationSeconds - 0.08) env = (durationSeconds - t) / 0.08;

        // Dynamic spectral brightness (harder blowing produces much brighter high harmonics)
        double brightness = 1.0 + 1.5 * velocity;

        double sample = 0.0;
        for (int k = 1; k <= numHarmonics; ++k) 
        {
            double fk = k * frequencyHz;
            if (fk >= sampleRate * 0.45) break;

            double harmonicWeight = std::pow(static_cast<double>(k), -1.2 / brightness);

            // Mute filter adjustments
            if (m_mute == BrassMuteType::Harmon) 
            {
                // High buzzing resonant peak
                harmonicWeight *= (k >= 3 && k <= 6) ? 2.2 : 0.4;
            } 
            else if 
            (m_mute == BrassMuteType::Straight) 
            {
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
    for (float& s : outputBuffer) 
    {
        s = std::tanh(s * 1.3f);
    }

    //m_musicSystem->RenderAudio(outputBuffer);
    m_musicSystem->MixAudioAsync(outputBuffer);
}

/**
 * @brief Plays multiple brass notes simultaneously as an ensemble chord.
 * @param frequencies Collection of fundamental frequencies in Hertz composing the chord.
 * @param durationSeconds Sounding duration in seconds.
 * @param velocity Blowing intensity scalar in the range [0.0, 1.0].
 */
void Trumpet::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) {
    for (double f : frequencies) {
        PlayNote(f, durationSeconds, velocity);
    }
}

/**
 * @brief Performs rapid staccato tonguing articulations on a single note.
 * @param frequencyHz Fundamental frequency in Hertz.
 * @param noteCount Total number of staccato pulses in the sequence.
 * @param noteDuration Duration of each individual note pulse in seconds.
 * @param velocity Strike intensity scalar in the range [0.0, 1.0].
 */
void Trumpet::Tonguing(double frequencyHz, int noteCount, double noteDuration, double velocity) {
    for (int i = 0; i < noteCount; ++i) 
    {
        PlayNote(frequencyHz, noteDuration, velocity);
    }
}

/**
 * @brief Plays a sequenced brass fanfare passage at a given tempo.
 * @details Computes beat intervals from tempoBpm and triggers each fanfare note with
 *          a 75% legato duty cycle and fixed high velocity (0.9).
 * @param notes Ordered vector of fundamental frequencies in Hertz representing the melody.
 * @param tempoBpm Tempo in beats per minute governing note durations.
 */
void Trumpet::PlayFanfare(const std::vector<double>& notes, double tempoBpm) {
    double beatSec = 60.0 / tempoBpm;
    
    for (double note : notes) 
    {
        PlayNote(note, beatSec * 0.75, 0.9);
    }
}