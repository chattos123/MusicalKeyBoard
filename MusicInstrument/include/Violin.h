#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IStringInstrument.h"
#include "IMusicSystem.h"
#include <string>
#include <vector>
#include <memory>

class MI_API Violin : public IMusicInstrument, public IStringInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem;
    std::string m_name;
    std::vector<double> m_openTuning; // Standard G3 (196.0), D4 (293.66), A4 (440.0), E5 (659.25)

public:
    explicit Violin(std::shared_ptr<IMusicSystem> musicSystem);
    ~Violin() override = default;

    // IMusicInstrument overrides
    std::string GetName() const override;
    void PlayNote(double frequencyHz, double durationSeconds = 1.5, double velocity = 0.8) override;
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.0, double velocity = 0.8) override;

    // IStringInstrument overrides
    int GetStringCount() const override;
    void PluckString(int stringIndex, double durationSeconds = 1.0, double velocity = 0.8) override; // Pizzicato
    void Strum(const std::vector<double>& chordFrequencies, double strumTimeMs = 20.0, double durationSeconds = 2.0) override;

    // Violin-specific: Continuous Bowing with vibrato (Arco)
    void BowString(double frequencyHz, double durationSeconds = 2.5, double bowPressure = 0.8, double vibratoDepthHz = 3.5, double vibratoRateHz = 5.5);
};