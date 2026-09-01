#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IStringInstrument.h"
#include "IMusicSystem.h"
#include <string>
#include <vector>
#include <memory>

class MI_API BassGuitar : public IMusicInstrument, public IStringInstrument {
private:
    std::shared_ptr<IMusicSystem> m_musicSystem;
    std::string m_name;
    std::vector<double> m_openTuning;

public:
    explicit BassGuitar(std::shared_ptr<IMusicSystem> musicSystem);
    ~BassGuitar() override = default;

    // IMusicInstrument implementation
    std::string GetName() const override;
    void PlayNote(double frequencyHz, double durationSeconds = 1.5, double velocity = 0.8) override;
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.0, double velocity = 0.8) override;

    // IStringInstrument implementation
    int GetStringCount() const override;
    void PluckString(int stringIndex, double durationSeconds = 1.5, double velocity = 0.8) override;
    void Strum(const std::vector<double>& chordFrequencies, double strumTimeMs = 30.0, double durationSeconds = 2.5) override;
};