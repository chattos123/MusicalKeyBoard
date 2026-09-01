#pragma once
#include "IMusicInstrument.h"
#include "IMusicSystem.h"
#include <memory>
#include <vector>

class MI_API Piano : public IMusicInstrument
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem;
    std::string m_name;

public:
    explicit Piano(std::shared_ptr<IMusicSystem> musicSystem);
    std::string GetName() const override;
    void PlayNote(double frequencyHz, double durationSeconds = 1.5, double velocity = 0.8) override;
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.5, double velocity = 0.8) override;
};