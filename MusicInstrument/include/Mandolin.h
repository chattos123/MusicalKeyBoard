#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IStringInstrument.h"
#include "IMusicSystem.h"
#include <string>
#include <vector>
#include <memory>

class MI_API Mandolin : public IMusicInstrument, public IStringInstrument {
private:
    std::shared_ptr<IMusicSystem> m_musicSystem;
    std::string m_name;
    // 4 courses of paired strings: G3 (196.00), D4 (293.66), A4 (440.00), E5 (659.25)
    std::vector<double> m_courseTunings;

public:
    explicit Mandolin(std::shared_ptr<IMusicSystem> musicSystem);
    ~Mandolin() override = default;

    // IMusicInstrument overrides
    std::string GetName() const override;
    void PlayNote(double frequencyHz, double durationSeconds = 1.0, double velocity = 0.8) override;
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 1.8, double velocity = 0.8) override;

    // IStringInstrument overrides
    int GetStringCount() const override; // 8 strings (4 paired courses)
    void PluckString(int courseIndex, double durationSeconds = 1.0, double velocity = 0.8) override;
    void Strum(const std::vector<double>& chordFrequencies, double strumTimeMs = 15.0, double durationSeconds = 1.8) override;

    // Mandolin-specific technique: Tremolo picking
    void PlayTremolo(double frequencyHz, double durationSeconds = 2.0, double rateHz = 12.0, double velocity = 0.8);
};