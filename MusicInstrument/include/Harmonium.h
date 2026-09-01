#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IMusicSystem.h"
#include <string>
#include <vector>
#include <memory>

class MI_API Harmonium : public IMusicInstrument {
private:
    std::shared_ptr<IMusicSystem> m_musicSystem;
    std::string m_name;
    bool m_couplerEnabled; // Dual-reed octave coupler (Bass + Treble reed banks)

public:
    explicit Harmonium(std::shared_ptr<IMusicSystem> musicSystem, bool enableCoupler = true);
    ~Harmonium() override = default;

    // IMusicInstrument overrides
    std::string GetName() const override;
    void PlayNote(double frequencyHz, double durationSeconds = 1.5, double velocity = 0.8) override;
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.5, double velocity = 0.8) override;

    // Harmonium-specific controls
    void SetCoupler(bool enabled);
    void PlayDrone(double rootFreqHz, double durationSeconds = 4.0, double velocity = 0.7);
};