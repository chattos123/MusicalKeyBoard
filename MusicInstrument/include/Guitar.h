/**
 * @file Guitar.h
 * @author Soumyajit Chattopadhyay
 * @date 03-Sep-2026
 * @brief Defines the Guitar class, a concrete implementation of IMusicInstrument and IStringInstrument.
 *
 * This file provides the Guitar class, which models a guitar instrument in the audio subsystem.
 * It integrates with an IMusicSystem backend to produce sound and exposes methods for playing notes,
 * chords, and string-specific actions such as plucking and strumming.
 *
 * Design Choices:
 * - Uses shared_ptr for IMusicSystem to ensure safe memory management and shared ownership.
 * - Implements both IMusicInstrument and IStringInstrument interfaces to provide general instrument
 *   behavior and string-specific functionality.
 * - Default parameters for duration and velocity are chosen to mimic realistic guitar playing styles.
 *
 * Physics Considerations:
 * - Frequencies represent the fundamental tones of guitar strings (typically E2, A2, D3, G3, B3, E4).
 * - Velocity approximates the force of plucking/strumming, influencing amplitude and harmonic richness.
 * - Duration models sustain time, reflecting string vibration decay and resonance.
 */

#pragma once
#include "IMusicInstrument.h"
#include "IStringInstrument.h"
#include "IMusicSystem.h"
#include <memory>
#include <vector>

/**
 * @class Guitar
 * @brief Represents a guitar instrument with string-specific behaviors.
 *
 * The Guitar class provides methods to play notes, chords, and perform string actions such as plucking
 * and strumming. It leverages the IMusicSystem backend to synthesize audio output.
 *
 * Key Features:
 * - Implements IMusicInstrument for general instrument playback.
 * - Implements IStringInstrument for string-specific actions.
 * - Supports open tuning configuration for realistic guitar modeling.
 */
class MI_API Guitar : public IMusicInstrument, public IStringInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem; ///< Backend audio system for sound synthesis
    std::string m_name;                          ///< Instrument name identifier
    std::vector<double> m_openTuning;            ///< Frequencies of open strings (Hz)

public:
    /**
     * @brief Constructs a Guitar instance.
     * @param musicSystem Shared pointer to the audio system backend.
     *
     * Design Choice: Dependency injection via shared_ptr ensures flexible integration
     * with different audio subsystems and safe memory management.
     */
    explicit Guitar(std::shared_ptr<IMusicSystem> musicSystem);

    // ---------------- IMusicInstrument Overrides ----------------

    /**
     * @brief Retrieves the instrument name.
     * @return Name of the instrument (e.g., "Guitar").
     */
    std::string GetName() const override;

    /**
     * @brief Plays a single note on the guitar.
     * @param frequencyHz Frequency of the note in Hertz.
     * @param durationSeconds Duration of the note in seconds (default: 1.0s).
     * @param velocity Intensity of the note (default: 0.8).
     *
     * Physics Note: Frequency maps to pitch, duration to sustain time, and velocity to amplitude.
     */
    void PlayNote(double frequencyHz, double durationSeconds = 1.0, double velocity = 0.8) override;

    /**
     * @brief Plays a chord consisting of multiple frequencies.
     * @param frequencies Vector of frequencies (Hz) representing chord notes.
     * @param durationSeconds Duration of the chord in seconds (default: 2.0s).
     * @param velocity Intensity of the chord (default: 0.8).
     *
     * Design Choice: Chords are modeled as simultaneous note playback.
     */
    void PlayChord(const std::vector<double>& frequencies,
                   double durationSeconds = 2.0,
                   double velocity = 0.8) override;

    // ---------------- IStringInstrument Overrides ----------------

    /**
     * @brief Retrieves the number of strings on the guitar.
     * @return Integer count of strings (typically 6).
     */
    int GetStringCount() const override;

    /**
     * @brief Plucks a specific string.
     * @param stringIndex Index of the string (0-based).
     * @param durationSeconds Duration of the note in seconds (default: 1.5s).
     * @param velocity Intensity of the pluck (default: 0.8).
     *
     * Physics Note: Plucking excites the fundamental frequency of the string,
     * with harmonics depending on velocity and damping.
     */
    void PluckString(int stringIndex,
                     double durationSeconds = 1.5,
                     double velocity = 0.8) override;

    /**
     * @brief Strums a chord across multiple strings.
     * @param chordFrequencies Frequencies representing the chord.
     * @param strumTimeMs Time in milliseconds between successive string hits (default: 25ms).
     * @param durationSeconds Duration of the strummed chord (default: 2.5s).
     *
     * Design Choice: Strumming introduces slight temporal offsets between notes,
     * mimicking realistic human playing.
     */
    void Strum(const std::vector<double>& chordFrequencies,
               double strumTimeMs = 25.0,
               double durationSeconds = 2.5) override;
};