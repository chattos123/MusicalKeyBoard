#pragma once
#include "MusicPlayerSystemExport.h"
#include <vector>

class MPS_API IMusicSystem 
{
public:
    virtual ~IMusicSystem() = default;
    virtual bool Setup() = 0;
    virtual void Clear() = 0;
    virtual double GetSampleRate() const = 0;
    virtual bool RenderAudio(const std::vector<float>& monoSamples) = 0;
    // Non-blocking real-time voice mixing interface
    virtual void MixAudioAsync(const std::vector<float>& monoSamples) = 0;
};