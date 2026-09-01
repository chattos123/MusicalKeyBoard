#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IWindInstrument.h"
#include "IMusicSystem.h"
#include <string>
#include <vector>
#include <memory>

class MI_API Trumpet : public IMusicInstrument, public IWindInstrument {
private:
    std::shared_ptr<IMusicSystem> m_musicSystem;
    std::string m_name;
    BrassMuteType m_mute;

public:
    explicit Trumpet(std::shared_ptr<IMusicSystem> musicSystem);
    ~Trumpet() override = default;

    // IMusicInstrument overrides
    std::string GetName() const override;
    void PlayNote(double frequencyHz, double durationSeconds = 1.2, double velocity = 0.8) override;
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.0, double velocity = 0.8) override;

    // IWindInstrument overrides
    void SetMute(BrassMuteType mute) override;
    void Tonguing(double frequencyHz, int noteCount, double noteDuration = 0.2, double velocity = 0.8) override;

    // Brass-specific fanfare articulation
    void PlayFanfare(const std::vector<double>& notes, double tempoBpm = 120.0);
};