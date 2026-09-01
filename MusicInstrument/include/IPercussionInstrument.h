#pragma once
#include "MusicInstrumentExport.h"
#include <string>
#include <vector>

enum class DrumPiece 
{
    BassDrum = 0,   // Kick drum (punchy low pitch-sweep)
    SnareDrum,      // Snare (body tone + white noise rattle)
    ClosedHiHat,    // Metallic short decay noise
    OpenHiHat,      // Metallic sustained noise
    LowTom,         // Resonant membrane
    HighTom,        // High membrane
    CrashCymbal     // Wide-spectrum exponential decay wash
};

class MI_API IPercussionInstrument 
{
public:
    virtual ~IPercussionInstrument() = default;
    
    // Core percussion capabilities
    virtual void HitDrum(DrumPiece piece, double velocity = 0.8, double durationSeconds = 1.0) = 0;
    virtual void PlayBeat(const std::vector<DrumPiece>& pieces, double durationSeconds = 1.0, double velocity = 0.8) = 0;
    virtual std::vector<std::string> GetDrumPieces() const = 0;
};