#pragma once

#include "IMusicInstrument.h"
#include "IWindInstrument.h"
#include "IMusicSystem.h"
#include "MusicInstrumentExport.h"
#include <memory>
#include <string>
#include <vector>

class MI_API Saxophone : public IMusicInstrument, public IWindInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_system;
    BrassMuteType m_currentMute{ BrassMuteType::None };

public:
    explicit Saxophone(std::shared_ptr<IMusicSystem> system);
    ~Saxophone() override = default;

    // IMusicInstrument Interface
    std::string GetName() const override;
    void PlayNote(double frequency, double duration, double velocity = 0.8) override;
    void PlayChord(const std::vector<double>& frequencies, double duration, double velocity = 0.8) override;

    // IWindInstrument Interface
    void SetMute(BrassMuteType mute) override;
    void Tonguing(double frequencyHz, int noteCount, double noteDuration = 0.2, double velocity = 0.8) override;
};