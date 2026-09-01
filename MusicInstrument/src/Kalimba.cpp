#include "Kalimba.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Kalimba::Kalimba(std::shared_ptr<IMusicSystem> musicSystem)
    : m_musicSystem(std::move(musicSystem)), m_name("17-Key Kalimba") 
{

}

std::string Kalimba::GetName() const 
{
    return m_name;
}

void Kalimba::PlayNote(double frequencyHz, double durationSeconds, double velocity) 
{
    PlayChord({ frequencyHz }, durationSeconds, velocity);
}

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