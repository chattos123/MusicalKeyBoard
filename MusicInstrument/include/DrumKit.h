#pragma once
#include "IMusicInstrument.h"
#include "IPercussionInstrument.h"
#include "IMusicSystem.h"
#include <memory>
#include <vector>

class MI_API DrumKit : public IMusicInstrument, public IPercussionInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem;
    std::string m_name;

    void SynthesizePiece(DrumPiece piece, double velocity, double sampleRate, std::vector<float>& buffer);

public:
    explicit DrumKit(std::shared_ptr<IMusicSystem> musicSystem);
    
    // IMusicInstrument Overrides
    std::string GetName() const override;
    void PlayNote(double frequencyHz, double durationSeconds = 1.0, double velocity = 0.8) override;
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.0, double velocity = 0.8) override;

    // IPercussionInstrument Overrides
    void HitDrum(DrumPiece piece, double velocity = 0.8, double durationSeconds = 1.0) override;
    void PlayBeat(const std::vector<DrumPiece>& pieces, double durationSeconds = 1.0, double velocity = 0.8) override;
    std::vector<std::string> GetDrumPieces() const override;
};