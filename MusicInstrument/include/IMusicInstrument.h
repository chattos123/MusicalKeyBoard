#pragma once
#include "MusicInstrumentExport.h"
#include <string>
#include <vector>

class MI_API IMusicInstrument 
{
public:
    virtual ~IMusicInstrument() = default;
    virtual std::string GetName() const = 0;
    virtual void PlayNote(double frequencyHz, double durationSeconds = 1.0, double velocity = 0.8) = 0;
    virtual void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.0, double velocity = 0.8) = 0;
};