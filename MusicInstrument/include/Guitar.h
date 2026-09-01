#pragma once
#include "IMusicInstrument.h"
#include "IStringInstrument.h"
#include "IMusicSystem.h"
#include <memory>
#include <vector>

class MI_API Guitar : public IMusicInstrument, public IStringInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem;
    std::string m_name;
    std::vector<double> m_openTuning;

public:
    explicit Guitar(std::shared_ptr<IMusicSystem> musicSystem);
    std::string GetName() const override;
    void PlayNote(double frequencyHz, double durationSeconds = 1.0, double velocity = 0.8) override;
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.0, double velocity = 0.8) override;

    int GetStringCount() const override;
    void PluckString(int stringIndex, double durationSeconds = 1.5, double velocity = 0.8) override;
    void Strum(const std::vector<double>& chordFrequencies, double strumTimeMs = 25.0, double durationSeconds = 2.5) override;
};