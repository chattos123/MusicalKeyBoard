#pragma once
#include "MusicInstrumentExport.h"
#include <vector>

class MI_API IStringInstrument 
{
public:
    virtual ~IStringInstrument() = default;
    virtual int GetStringCount() const = 0;
    virtual void PluckString(int stringIndex, double durationSeconds = 1.5, double velocity = 0.8) = 0;
    virtual void Strum(const std::vector<double>& chordFrequencies, double strumTimeMs = 25.0, double durationSeconds = 2.5) = 0;
};