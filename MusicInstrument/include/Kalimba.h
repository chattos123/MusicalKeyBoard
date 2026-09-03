/**
 * @file Kalimba.h
 * @author Soumyajit Chattopadhyay
 * @date 03-Sep-2026
 * @brief Defines the Kalimba class, a concrete implementation of IMusicInstrument.
 *
 * This file provides the Kalimba class, which models a kalimba (thumb piano) instrument in the audio subsystem.
 * It integrates with an IMusicSystem backend to produce sound and exposes methods for playing notes and chords.
 *
 * Design Choices:
 * - Uses shared_ptr for IMusicSystem to ensure safe memory management and shared ownership.
 * - Implements IMusicInstrument for general instrument playback.
 * - Default parameters for duration and velocity are chosen to mimic realistic kalimba playing styles.
 *
 * Physics Considerations:
 * - Frequencies represent the fundamental tones of kalimba tines (metal keys).
 * - Velocity approximates the force of plucking a tine, influencing amplitude and harmonic richness.
 * - Duration models sustain time, reflecting vibration decay of the tine.
 */

#pragma once
#include "IMusicInstrument.h"
#include "IMusicSystem.h"
#include <memory>

/**
 * @class Kalimba
 * @brief Represents a kalimba instrument with tine-specific behaviors.
 *
 * The Kalimba class provides methods to play notes and chords. It leverages the IMusicSystem backend
 * to synthesize audio output.
 *
 * Key Features:
 * - Implements IMusicInstrument for general instrument playback.
 * - Models kalimba-specific sound characteristics (short sustain, bright timbre).
 */
class MI_API Kalimba : public IMusicInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem; ///< Backend audio system for sound synthesis
    std::string m_name;                          ///< Instrument name identifier

public:
    /**
     * @brief Constructs a Kalimba instance.
     * @param musicSystem Shared pointer to the audio system backend.
     *
     * Design Choice: Dependency injection via shared_ptr ensures flexible integration
     * with different audio subsystems and safe memory management.
     */
    explicit Kalimba(std::shared_ptr<IMusicSystem> musicSystem);

    // ---------------- IMusicInstrument Overrides ----------------

    /**
     * @brief Retrieves the instrument name.
     * @return Name of the instrument (e.g., "Kalimba").
     */
    std::string GetName() const override;

    /**
     * @brief Plays a single note on the kalimba.
     * @param frequencyHz Frequency of the note in Hertz.
     * @param durationSeconds Duration of the note in seconds (default: 1.2s).
     * @param velocity Intensity of the note (default: 0.8).
     *
     * Physics Note: Frequency maps to tine pitch, duration to sustain time, and velocity to plucking force.
     */
    void PlayNote(double frequencyHz, double durationSeconds = 1.2, double velocity = 0.8) override;

    /**
     * @brief Plays a chord consisting of multiple frequencies.
     * @param frequencies Vector of frequencies (Hz) representing chord notes.
     * @param durationSeconds Duration of the chord in seconds (default: 2.0s).
     * @param velocity Intensity of the chord (default: 0.8).
     *
     * Design Choice: Chords are modeled as simultaneous tine plucks, producing bright harmonic blends.
     */
    void PlayChord(const std::vector<double>& frequencies,
                   double durationSeconds = 2.0,
                   double velocity = 0.8) override;
};