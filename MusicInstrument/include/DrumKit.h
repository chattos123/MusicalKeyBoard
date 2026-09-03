/**
 * @file DrumKit.h
 * @author Soumyajit Chattopadhyay
 * @date 03-Sep-2026
 * @brief Defines the DrumKit class, a concrete implementation of IMusicInstrument and IPercussionInstrument.
 *
 * This file provides the DrumKit class, which models a digital drum kit instrument in the audio subsystem.
 * It integrates with an IMusicSystem backend to synthesize percussion sounds and exposes methods for
 * hitting drums, playing beats, and retrieving drum piece metadata.
 *
 * Design Choices:
 * - Uses shared_ptr for IMusicSystem to ensure safe memory management and shared ownership.
 * - Implements both IMusicInstrument and IPercussionInstrument interfaces to provide general instrument
 *   behavior and percussion-specific functionality.
 * - Encapsulates synthesis logic in a private helper method (SynthesizePiece) for modularity and reuse.
 *
 * Physics Considerations:
 * - Percussion instruments produce transient, non-pitched sounds; frequency is less relevant than
 *   amplitude envelope and timbre.
 * - Velocity approximates striking force, influencing loudness and harmonic content.
 * - Duration models sustain time, reflecting damping and resonance of drum skins.
 */

#pragma once
#include "IMusicInstrument.h"
#include "IPercussionInstrument.h"
#include "IMusicSystem.h"
#include <memory>
#include <vector>

/**
 * @class DrumKit
 * @brief Represents a drum kit instrument with percussion-specific behaviors.
 *
 * The DrumKit class provides methods to hit individual drum pieces, play beats, and synthesize
 * percussion sounds. It leverages the IMusicSystem backend to generate audio output.
 *
 * Key Features:
 * - Implements IMusicInstrument for general instrument playback.
 * - Implements IPercussionInstrument for percussion-specific actions.
 * - Supports multiple drum pieces (snare, kick, hi-hat, etc.) with configurable velocity and duration.
 */
class MI_API DrumKit : public IMusicInstrument, public IPercussionInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem; ///< Backend audio system for sound synthesis
    std::string m_name;                          ///< Instrument name identifier

    /**
     * @brief Synthesizes audio for a specific drum piece.
     * @param piece Enum representing the drum piece (e.g., snare, kick).
     * @param velocity Intensity of the hit (0.0–1.0).
     * @param sampleRate Sampling rate for audio buffer.
     * @param buffer Output buffer to hold synthesized audio samples.
     *
     * Physics Note: Percussion synthesis focuses on transient attack, resonance, and decay.
     */
    void SynthesizePiece(DrumPiece piece, double velocity, double sampleRate, std::vector<float>& buffer);

public:
    /**
     * @brief Constructs a DrumKit instance.
     * @param musicSystem Shared pointer to the audio system backend.
     *
     * Design Choice: Dependency injection via shared_ptr ensures flexible integration
     * with different audio subsystems and safe memory management.
     */
    explicit DrumKit(std::shared_ptr<IMusicSystem> musicSystem);
    
    // ---------------- IMusicInstrument Overrides ----------------

    /**
     * @brief Retrieves the instrument name.
     * @return Name of the instrument (e.g., "Drum Kit").
     */
    std::string GetName() const override;

    /**
     * @brief Plays a single percussion sound.
     * @param frequencyHz Frequency parameter (not typically used for percussion).
     * @param durationSeconds Duration of the sound in seconds (default: 1.0s).
     * @param velocity Intensity of the hit (default: 0.8).
     *
     * Physics Note: Percussion sounds are broadband; frequency parameter may be ignored
     * or mapped to timbre variation.
     */
    void PlayNote(double frequencyHz, double durationSeconds = 1.0, double velocity = 0.8) override;

    /**
     * @brief Plays a chord-like combination of percussion sounds.
     * @param frequencies Vector of pseudo-frequencies (mapped to drum pieces).
     * @param durationSeconds Duration of the combined sound (default: 2.0s).
     * @param velocity Intensity of the combined hit (default: 0.8).
     *
     * Design Choice: Chords are modeled as simultaneous percussion hits.
     */
    void PlayChord(const std::vector<double>& frequencies,
                   double durationSeconds = 2.0,
                   double velocity = 0.8) override;

    // ---------------- IPercussionInstrument Overrides ----------------

    /**
     * @brief Hits a specific drum piece.
     * @param piece Enum representing the drum piece.
     * @param velocity Intensity of the hit (default: 0.8).
     * @param durationSeconds Duration of the sound (default: 1.0s).
     *
     * Physics Note: Velocity influences amplitude and harmonic richness.
     */
    void HitDrum(DrumPiece piece, double velocity = 0.8, double durationSeconds = 1.0) override;

    /**
     * @brief Plays a beat consisting of multiple drum pieces.
     * @param pieces Vector of drum pieces to hit.
     * @param durationSeconds Duration of the beat (default: 1.0s).
     * @param velocity Intensity of the beat (default: 0.8).
     *
     * Design Choice: Beats are modeled as sequential or simultaneous hits depending on implementation.
     */
    void PlayBeat(const std::vector<DrumPiece>& pieces,
                  double durationSeconds = 1.0,
                  double velocity = 0.8) override;

    /**
     * @brief Retrieves the names of available drum pieces.
     * @return Vector of strings representing drum piece names.
     */
    std::vector<std::string> GetDrumPieces() const override;
};