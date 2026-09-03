/**
 * @file Mandolin.h
 * @author Soumyajit Chattopadhyay
 * @date 03-Sep-2026
 * @brief Defines the Mandolin class, a concrete implementation of IMusicInstrument and IStringInstrument.
 *
 * This file provides the Mandolin class, which models a mandolin instrument in the audio subsystem.
 * It integrates with an IMusicSystem backend to produce sound and exposes methods for playing notes,
 * chords, and string-specific actions such as plucking, strumming, and tremolo picking.
 *
 * Design Choices:
 * - Uses shared_ptr for IMusicSystem to ensure safe memory management and shared ownership.
 * - Implements both IMusicInstrument and IStringInstrument interfaces to provide general instrument
 *   behavior and string-specific functionality.
 * - Default parameters for duration and velocity are chosen to mimic realistic mandolin playing styles.
 * - Supports tremolo picking, a signature technique of mandolin performance.
 *
 * Physics Considerations:
 * - Mandolin has 4 courses of paired strings (8 total), tuned G3, D4, A4, E5.
 * - Frequencies represent the fundamental tones of paired strings, producing richer harmonics.
 * - Velocity approximates the force of plucking/strumming, influencing amplitude and brightness.
 * - Duration models sustain time, reflecting string vibration decay.
 * - Tremolo simulates rapid alternation of strokes, sustaining notes beyond natural decay.
 */

#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IStringInstrument.h"
#include "IMusicSystem.h"
#include <string>
#include <vector>
#include <memory>

/**
 * @class Mandolin
 * @brief Represents a mandolin instrument with string-specific behaviors.
 *
 * The Mandolin class provides methods to play notes, chords, and perform string actions such as plucking,
 * strumming, and tremolo picking. It leverages the IMusicSystem backend to synthesize audio output.
 *
 * Key Features:
 * - Implements IMusicInstrument for general instrument playback.
 * - Implements IStringInstrument for string-specific actions.
 * - Supports tremolo picking to simulate sustained notes.
 * - Models realistic mandolin tuning with 4 paired courses.
 */
class MI_API Mandolin : public IMusicInstrument, public IStringInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem; ///< Backend audio system for sound synthesis
    std::string m_name;                          ///< Instrument name identifier
    std::vector<double> m_courseTunings;         ///< Frequencies of paired string courses (Hz)

public:
    /**
     * @brief Constructs a Mandolin instance.
     * @param musicSystem Shared pointer to the audio system backend.
     *
     * Design Choice: Dependency injection via shared_ptr ensures flexible integration
     * with different audio subsystems and safe memory management.
     */
    explicit Mandolin(std::shared_ptr<IMusicSystem> musicSystem);

    /**
     * @brief Default destructor.
     */
    ~Mandolin() override = default;

    // ---------------- IMusicInstrument Overrides ----------------

    /**
     * @brief Retrieves the instrument name.
     * @return Name of the instrument (e.g., "Mandolin").
     */
    std::string GetName() const override;

    /**
     * @brief Plays a single note on the mandolin.
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
     * @param durationSeconds Duration of the chord in seconds (default: 1.8s).
     * @param velocity Intensity of the chord (default: 0.8).
     *
     * Design Choice: Chords are modeled as simultaneous note playback across paired courses.
     */
    void PlayChord(const std::vector<double>& frequencies,
                   double durationSeconds = 1.8,
                   double velocity = 0.8) override;

    // ---------------- IStringInstrument Overrides ----------------

    /**
     * @brief Retrieves the number of strings on the mandolin.
     * @return Integer count of strings (8 total, 4 paired courses).
     */
    int GetStringCount() const override;

    /**
     * @brief Plucks a specific course of paired strings.
     * @param courseIndex Index of the course (0-based).
     * @param durationSeconds Duration of the note in seconds (default: 1.0s).
     * @param velocity Intensity of the pluck (default: 0.8).
     *
     * Physics Note: Plucking excites both strings in a course, producing richer harmonics.
     */
    void PluckString(int courseIndex,
                     double durationSeconds = 1.0,
                     double velocity = 0.8) override;

    /**
     * @brief Strums a chord across multiple courses.
     * @param chordFrequencies Frequencies representing the chord.
     * @param strumTimeMs Time in milliseconds between successive string hits (default: 15ms).
     * @param durationSeconds Duration of the strummed chord (default: 1.8s).
     *
     * Design Choice: Strumming introduces slight temporal offsets between notes,
     * mimicking realistic human playing.
     */
    void Strum(const std::vector<double>& chordFrequencies,
               double strumTimeMs = 15.0,
               double durationSeconds = 1.8) override;

    // ---------------- Mandolin-Specific Technique ----------------

    /**
     * @brief Plays a tremolo on a single note.
     * @param frequencyHz Frequency of the note in Hertz.
     * @param durationSeconds Duration of the tremolo in seconds (default: 2.0s).
     * @param rateHz Tremolo rate in Hertz (default: 12.0 Hz).
     * @param velocity Intensity of the tremolo strokes (default: 0.8).
     *
     * Physics Note: Tremolo simulates rapid alternation of strokes, sustaining notes beyond natural decay.
     */
    void PlayTremolo(double frequencyHz,
                     double durationSeconds = 2.0,
                     double rateHz = 12.0,
                     double velocity = 0.8);
};