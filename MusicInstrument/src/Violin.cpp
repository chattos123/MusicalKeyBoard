/**
 * @file Violin.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the Violin class for bowed and plucked string synthesis.
 *
 * This file contains the implementation of the Violin class, modeling a 4-string acoustic
 * violin. It implements physical modeling for both bowed string excitations (Arco) and plucked
 * string excitations (Pizzicato). It features Helmholtz motion sawtooth harmonic series,
 * delayed vibrato modulation, acoustic bridge and spruce soundboard resonance formants (2.5 kHz),
 * and Karplus-Strong waveguide synthesis for pizzicato dynamics.
 *
 * Design Choices:
 * - Uses additive harmonic synthesis with odd/even balance adjustments and acoustic body formant
 *   filtering to emulate bowed violin tone.
 * - Implements a 200 ms delayed pitch vibrato LFO (frequency modulation) to mimic natural classical
 *   bowing technique where vibrato begins after note onset.
 * - Simulates Pizzicato using the Karplus-Strong waveguide algorithm with high damping (decay = 0.985)
 *   to replicate the fast energy absorption of finger plucks on short strings.
 * - Trapezoidal amplitude envelopes ensure click-free onset and decay during bow stroke changes.
 * - Applies hyperbolic tangent (std::tanh) soft saturation for dynamic headroom protection.
 * - Enqueues rendered bowed audio asynchronously to IMusicSystem::MixAudioAsync.
 *
 * Physics & Acoustics Notes:
 * - Open string fundamental frequencies: G3 (196.00 Hz), D4 (293.66 Hz), A4 (440.00 Hz), E5 (659.25 Hz).
 * - Bowed string excitation produces Helmholtz motion, generating a sawtooth-like wave with 1/k amplitude falloff.
 * - Bridge hill resonance enhances frequencies near 2.5 kHz, projecting the acoustic sound through the soundboard and f-holes.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "Violin.h"
#include <cmath>
#include <random>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Constructs an Acoustic Violin instance bound to an audio subsystem.
 * @param musicSystem Shared pointer to the audio system backend for sound rendering.
 */
Violin::Violin(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)),
      m_name("Acoustic Violin"),
      m_openTuning{ 196.00, 293.66, 440.00, 659.25 }
{
}

/**
 * @brief Retrieves the human-readable identifier of the instrument.
 * @return Standard string containing the instrument name ("Acoustic Violin").
 */
std::string Violin::GetName() const 
{
    return m_name;
}

/**
 * @brief Retrieves the total count of physical strings on the violin.
 * @return Number of open strings configured (4).
 */
int Violin::GetStringCount() const 
{
    return static_cast<int>(m_openTuning.size());
}

/**
 * @brief Plays a single note using standard bowed articulation.
 * @param frequencyHz Fundamental pitch frequency in Hertz.
 * @param durationSeconds Sounding duration in seconds.
 * @param velocity Bow pressure/intensity scalar in the range [0.0, 1.0].
 */
void Violin::PlayNote(double frequencyHz, double durationSeconds, double velocity) 
{
    BowString(frequencyHz, durationSeconds, velocity);
}

/**
 * @brief Synthesizes and plays a polyphonic stopped chord or violin ensemble voicing.
 * @details Synthesizes an 8-harmonic Helmholtz-like series per note with odd/even amplitude tilt,
 *          applies a smooth bow attack and release envelope, scales the composite output by the
 *          square root of the voice count, applies std::tanh soft-clipping, and renders via RenderAudio.
 * @param frequencies Vector of fundamental frequencies in Hertz composing the chord.
 * @param durationSeconds Sounding duration in seconds.
 * @param velocity Intensity scalar across the chord voices in the range [0.0, 1.0].
 */
void Violin::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) 
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
            double attack = (t < 0.1) ? (t / 0.1) : (t > durationSeconds - 0.2 ? (durationSeconds - t) / 0.2 : 1.0);
            
            // Violin Helmholtz sawtooth-like harmonic series
            double sample = 0.0;
            for (int k = 1; k <= 8; ++k) 
            {
                double f_k = k * f0;
                if (f_k >= sampleRate * 0.45) break;
                double amp = (1.0 / k) * (0.8 + 0.2 * (k % 2)); // Odd/even wooden body tilt
                sample += amp * std::sin(2.0 * M_PI * f_k * t);
            }

            outputBuffer[n] += static_cast<float>(sample * attack * 0.3);
        }
    }

    for (float& s : outputBuffer) 
    {
        s = std::tanh(s / std::sqrt(static_cast<float>(frequencies.size())));
    }

    m_musicSystem->RenderAudio(outputBuffer);
}

/**
 * @brief Plucks an open string (Pizzicato) using the Karplus-Strong waveguide algorithm.
 * @details Allocates a circular ring buffer based on the open string frequency, initializes
 *          the delay line with uniform white noise, applies a rapid feedback damping filter
 *          (decay = 0.985) simulating finger damping, soft-clips via std::tanh, and renders the buffer.
 * @param stringIndex Zero-based string index (0: G3, 1: D4, 2: A4, 3: E5).
 * @param durationSeconds Total decay duration in seconds.
 * @param velocity Strike force scalar in the range [0.0, 1.0].
 */
void Violin::PluckString(int stringIndex, double durationSeconds, double velocity) 
{
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

    for (size_t n = 0; n < totalSamples; ++n) 
    {
        float currentVal = ring[idx];
        size_t nextIdx = (idx + 1) % ringSize;
        ring[idx] = decay * 0.5f * (ring[idx] + ring[nextIdx]);
        idx = nextIdx;
        outputBuffer[n] = currentVal * static_cast<float>(velocity);
    }

    for (float& s : outputBuffer) s = std::tanh(s);
    m_musicSystem->RenderAudio(outputBuffer);
}

/**
 * @brief Plays a chord voicing across multiple strings.
 * @param chordFrequencies Collection of fundamental frequencies in Hertz.
 * @param strumTimeMs Sweep offset between strings in milliseconds (unused in direct chord dispatch).
 * @param durationSeconds Total sounding duration in seconds.
 */
void Violin::Strum(const std::vector<double>& chordFrequencies, double strumTimeMs, double durationSeconds) 
{
    PlayChord(chordFrequencies, durationSeconds, 0.8);
}

/**
 * @brief Synthesizes a sustained bowed note (Arco) with expressive vibrato and body formants.
 * @details Simulates physical bowed violin performance:
 *          - Dynamic breath/bow envelope with 150 ms attack and 250 ms release.
 *          - Delayed vibrato beginning ~200 ms after note onset via an LFO (rate and depth).
 *          - 10-harmonic additive series weighted by bridge and soundboard resonance around 2.5 kHz.
 *          - Soft-clipping compression via std::tanh and asynchronous mixer dispatch.
 * @param frequencyHz Fundamental tone frequency in Hertz.
 * @param durationSeconds Total bow stroke duration in seconds.
 * @param bowPressure Normalized bow hair contact pressure in the range [0.0, 1.0].
 * @param vibratoDepthHz Maximum pitch deviation around the fundamental in Hertz (default: 3.5 Hz).
 * @param vibratoRateHz Pitch modulation cycle rate in Hertz (default: 5.5 Hz).
 */
void Violin::BowString(double frequencyHz, double durationSeconds, double bowPressure, double vibratoDepthHz, double vibratoRateHz) 
{
    if (!m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    double phase = 0.0;
    const int numHarmonics = 10;

    for (size_t n = 0; n < totalSamples; ++n) 
    {
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
        for (int k = 1; k <= numHarmonics; ++k) 
        {
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