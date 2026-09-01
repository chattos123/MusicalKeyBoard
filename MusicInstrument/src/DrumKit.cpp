#include "DrumKit.h"
#include <cmath>
#include <random>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DrumKit::DrumKit(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)), m_name("Standard Acoustic Drum Kit") {}

std::string DrumKit::GetName() const 
{
    return m_name;
}

std::vector<std::string> DrumKit::GetDrumPieces() const 
{
    return { "Bass Drum (Kick)", "Snare Drum", "Closed Hi-Hat", "Open Hi-Hat", "Low Tom", "High Tom", "Crash Cymbal" };
}

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

void DrumKit::HitDrum(DrumPiece piece, double velocity, double durationSeconds) 
{
    PlayBeat({ piece }, durationSeconds, velocity);
}

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

// IMusicInstrument fallback implementation (maps pitch to closest drum hit)
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