/**
 * @file DrumKit.cpp
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Implementation of the DrumKit class for acoustic percussion synthesis.
 *
 * This file provides the concrete implementation of the DrumKit class, synthesizing
 * acoustic drum membrane, shell, and metallic percussion sounds using procedural DSP techniques.
 * It models physical characteristics such as pitch envelopes, resonant decay, beater impact clicks,
 * snare wire rattles, and metallic cymbals, outputting directly to the audio subsystem.
 *
 * Design Choices:
 * - Uses procedural mathematical functions (sine oscillators, exponential envelopes, pseudo-random noise)
 *   to avoid external audio asset/sample dependencies.
 * - Polyphonic beat layering dynamically scales and soft-clips multi-voice hits using std::tanh.
 * - Bridges melodic calls (PlayNote, PlayChord) from the IMusicInstrument interface to percussion
 *   elements via frequency threshold partitioning.
 * - Asynchronous hardware mixing is dispatched through IMusicSystem::MixAudioAsync.
 *
 * Physics & Synthesis Notes:
 * - Bass Drum (Kick): Exponential sweep dropping from 150 Hz to 45 Hz with a brief 5 ms beater click.
 * - Snare Drum: 180 Hz exponentially decaying shell fundamental blended with noise representing snare wires.
 * - Hi-Hats: Steep exponential decays of white noise simulating fast mechanical damping (closed) or sustained cymbal wash (open).
 * - Toms: Resonant decaying membranes with moderate initial pitch drops.
 * - Crash Cymbal: Extended exponential decay over high-frequency noise wash.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#include "DrumKit.h"
#include <cmath>
#include <random>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Constructs a DrumKit instance bound to an audio subsystem.
 * @param musicSystem Shared pointer to the audio system backend for sound rendering.
 */
DrumKit::DrumKit(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)), m_name("Standard Acoustic Drum Kit") {}

/**
 * @brief Retrieves the human-readable identifier of the drum kit.
 * @return Standard string containing the drum kit name.
 */
std::string DrumKit::GetName() const 
{
    return m_name;
}

/**
 * @brief Retrieves the descriptive names of all supported drum kit pieces.
 * @return Vector of string identifiers corresponding to available drum pieces.
 */
std::vector<std::string> DrumKit::GetDrumPieces() const 
{
    return { "Bass Drum (Kick)", "Snare Drum", "Closed Hi-Hat", "Open Hi-Hat", "Low Tom", "High Tom", "Crash Cymbal" };
}

/**
 * @brief Procedurally synthesizes a single percussion piece into an audio buffer.
 * @details Computes time-domain samples based on physical acoustic principles:
 *          - Bass Drum: Fast pitch drop coupled with transient beater noise.
 *          - Snare Drum: Decaying body resonance mixed with wire rattle noise.
 *          - Closed Hi-Hat: Metallic noise with rapid exponential decay (60/sec).
 *          - Open Hi-Hat: Sustained metallic noise with moderate decay (8/sec).
 *          - Toms: Low/High resonant membrane pitch slides.
 *          - Crash Cymbal: Dense broad-spectrum noise with slow decay (2.5/sec).
 * @param piece The DrumPiece enum value specifying the drum component to synthesize.
 * @param velocity Strike velocity/intensity scalar in the range [0.0, 1.0].
 * @param sampleRate Audio sampling rate in Hertz.
 * @param buffer Output PCM sample buffer to which the synthesized piece is accumulated.
 */
void DrumKit::SynthesizePiece(DrumPiece piece, double velocity, double sampleRate, std::vector<float>& buffer) 
{
    size_t totalSamples = buffer.size();
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> noiseDist(-1.0f, 1.0f);

    double phase = 0.0;

    for (size_t n = 0; n < totalSamples; ++n) 
    {
        double t = static_cast<double>(n) / sampleRate;
        float sample = 0.0f;

        switch (piece) 
        {
        case DrumPiece::BassDrum: 
        {
            // Kick: Fast pitch envelope drop from 150 Hz to 45 Hz
            double freq = 45.0 + 105.0 * std::exp(-40.0 * t);
            double env = std::exp(-8.0 * t);
            phase += 2.0 * M_PI * freq / sampleRate;
            sample = static_cast<float>(std::sin(phase) * env);
            // Click on beater impact
            if (t < 0.005) sample += static_cast<float>(noiseDist(rng) * (1.0 - t / 0.005) * 0.4);
            break;
        }
        case DrumPiece::SnareDrum: 
        {
            // Snare: Tone (180 Hz) + noise wire burst
            double freq = 180.0 * std::exp(-15.0 * t);
            phase += 2.0 * M_PI * freq / sampleRate;
            double tone = std::sin(phase) * std::exp(-15.0 * t);
            double noise = noiseDist(rng) * std::exp(-20.0 * t);
            sample = static_cast<float>(0.4 * tone + 0.6 * noise);
            break;
        }
        case DrumPiece::ClosedHiHat: {
            // High frequency metallic snap
            double noise = noiseDist(rng);
            double env = std::exp(-60.0 * t);
            sample = static_cast<float>(noise * env * 0.7);
            break;
        }
        case DrumPiece::OpenHiHat: 
        {
            // Sustained metallic wash
            double noise = noiseDist(rng);
            double env = std::exp(-8.0 * t);
            sample = static_cast<float>(noise * env * 0.6);
            break;
        }
        case DrumPiece::LowTom: 
        {
            double freq = 85.0 + 40.0 * std::exp(-20.0 * t);
            double env = std::exp(-6.0 * t);
            phase += 2.0 * M_PI * freq / sampleRate;
            sample = static_cast<float>(std::sin(phase) * env);
            break;
        }
        case DrumPiece::HighTom: 
        {
            double freq = 140.0 + 50.0 * std::exp(-20.0 * t);
            double env = std::exp(-7.0 * t);
            phase += 2.0 * M_PI * freq / sampleRate;
            sample = static_cast<float>(std::sin(phase) * env);
            break;
        }
        case DrumPiece::CrashCymbal: 
        {
            double noise = noiseDist(rng);
            double env = std::exp(-2.5 * t);
            sample = static_cast<float>(noise * env * 0.8);
            break;
        }
        }

        buffer[n] += sample * static_cast<float>(velocity);
    }
}

/**
 * @brief Strikes a single drum piece and plays it through the audio subsystem.
 * @param piece The DrumPiece enum value specifying the component to strike.
 * @param velocity Strike intensity scalar in the range [0.0, 1.0] (default: 0.8).
 * @param durationSeconds Duration of the rendered audio segment in seconds (default: 1.0s).
 */
void DrumKit::HitDrum(DrumPiece piece, double velocity, double durationSeconds) 
{
    PlayBeat({ piece }, durationSeconds, velocity);
}

/**
 * @brief Synthesizes and plays multiple drum pieces simultaneously as a layered composite beat.
 * @details Synthesizes each specified piece into a common output buffer, normalizes the composite
 *          amplitude by the square root of the piece count, applies hyperbolic tangent (tanh)
 *          soft limiting to prevent clipping, and routes the buffer to the asynchronous mixer.
 * @param pieces Vector of DrumPiece values to strike simultaneously.
 * @param durationSeconds Total sound duration in seconds (default: 1.0s).
 * @param velocity Strike intensity scalar in the range [0.0, 1.0] (default: 0.8).
 */
void DrumKit::PlayBeat(const std::vector<DrumPiece>& pieces, double durationSeconds, double velocity) 
{
    if (pieces.empty() || !m_musicSystem) return;

    double sampleRate = m_musicSystem->GetSampleRate();
    size_t totalSamples = static_cast<size_t>(durationSeconds * sampleRate);
    std::vector<float> outputBuffer(totalSamples, 0.0f);

    for (DrumPiece piece : pieces) {
        SynthesizePiece(piece, velocity, sampleRate, outputBuffer);
    }

    for (float& s : outputBuffer) {
        s = std::tanh(s / std::sqrt(static_cast<float>(pieces.size())));
    }

    //m_musicSystem->RenderAudio(outputBuffer);
    m_musicSystem->MixAudioAsync(outputBuffer);
}

/**
 * @brief IMusicInstrument melodic fallback mapping pitch frequency to an appropriate drum piece.
 * @details Partitions the audio spectrum to trigger percussion pieces based on input frequency:
 *          - < 80 Hz: Bass Drum (Kick)
 *          - 80 Hz - 159 Hz: Low Tom
 *          - 160 Hz - 239 Hz: Snare Drum
 *          - >= 240 Hz: Closed Hi-Hat
 * @param frequencyHz Input frequency in Hertz.
 * @param durationSeconds Duration of the sound in seconds.
 * @param velocity Intensity scalar in the range [0.0, 1.0].
 */
void DrumKit::PlayNote(double frequencyHz, double durationSeconds, double velocity)
{
    if (frequencyHz < 80.0) 
    {
        HitDrum(DrumPiece::BassDrum, velocity, durationSeconds);
    } 
    else if (frequencyHz < 160.0) 
    {
        HitDrum(DrumPiece::LowTom, velocity, durationSeconds);
    } 
    else if (frequencyHz < 240.0) 
    {
        HitDrum(DrumPiece::SnareDrum, velocity, durationSeconds);
    } 
    else 
    {
        HitDrum(DrumPiece::ClosedHiHat, velocity, durationSeconds);
    }
}

/**
 * @brief IMusicInstrument melodic fallback mapping a chord of frequencies into a composite beat.
 * @details Analyzes input frequencies and groups them into drum pieces:
 *          - < 100 Hz: Bass Drum
 *          - 100 Hz - 199 Hz: Snare Drum
 *          - >= 200 Hz: Closed Hi-Hat
 *          Triggers all mapped pieces simultaneously via PlayBeat.
 * @param frequencies Vector of frequencies in Hertz representing the chord.
 * @param durationSeconds Duration of the resulting sound in seconds.
 * @param velocity Intensity scalar in the range [0.0, 1.0].
 */
void DrumKit::PlayChord(const std::vector<double>& frequencies, double durationSeconds, double velocity) 
{
    std::vector<DrumPiece> pieces;

    for (double f : frequencies) 
    {
        if (f < 100.0) pieces.push_back(DrumPiece::BassDrum);
        else if (f < 200.0) pieces.push_back(DrumPiece::SnareDrum);
        else pieces.push_back(DrumPiece::ClosedHiHat);
    }

    PlayBeat(pieces, durationSeconds, velocity);
}