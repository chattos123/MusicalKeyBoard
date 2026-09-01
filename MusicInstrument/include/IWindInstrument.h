#pragma once

#include "MusicInstrumentExport.h"
#include <string>

enum class BrassMuteType 
{
    None = 0,
    Straight,
    Cup,
    Harmon
};

class MI_API IWindInstrument 
{
public:
    virtual ~IWindInstrument() = default;

    // Wind-specific techniques
    virtual void SetMute(BrassMuteType mute) = 0;
    virtual void Tonguing(double frequencyHz, int noteCount, double noteDuration = 0.2, double velocity = 0.8) = 0;
};