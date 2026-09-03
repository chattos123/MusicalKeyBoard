/**
 * @file Harmonium.h
 * @author Soumyajit Chattopadhyay
 * @date 03-Sep-2026
 * @brief Defines the Harmonium class, a concrete implementation of IMusicInstrument.
 *
 * This file provides the Harmonium class, which models a harmonium instrument in the audio subsystem.
 * It integrates with an IMusicSystem backend to produce sound and exposes methods for playing notes,
 * chords, and harmonium-specific features such as coupler control and drone playback.
 *
 * Design Choices:
 * - Uses shared_ptr for IMusicSystem to ensure safe memory management and shared ownership.
 * - Implements IMusicInstrument for general instrument playback.
 * - Adds harmonium-specific features like octave coupler and drone to reflect authentic playing styles.
 * - Default parameters for duration and velocity are chosen to mimic realistic harmonium performance.
 *
 * Physics Considerations:
 * - Frequencies represent the fundamental tones produced by reed banks.
 * - Velocity approximates airflow pressure applied via bellows, influencing amplitude and timbre.
 * - Duration models sustain time, reflecting reed vibration and airflow consistency.
 * - Coupler enables dual-reed octave doubling (bass + treble), enriching harmonic texture.
 */

#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IMusicSystem.h"
#include <string>
#include <vector>
#include <memory>

/**
 * @class Harmonium
 * @brief Represents a harmonium instrument with reed and coupler-specific behaviors.
 *
 * The Harmonium class provides methods to play notes, chords, and drones, while supporting
 * harmonium-specific features such as octave coupler control. It leverages the IMusicSystem
 * backend to synthesize audio output.
 *
 * Key Features:
 * - Implements IMusicInstrument for general instrument playback.
 * - Supports coupler functionality for octave doubling.
 * - Provides drone playback to simulate sustained root notes common in Indian classical music.
 */
class MI_API Harmonium : public IMusicInstrument {
private:
    std::shared_ptr<IMusicSystem> m_musicSystem; ///< Backend audio system for sound synthesis
    std::string m_name;                          ///< Instrument name identifier
    bool m_couplerEnabled;                       ///< Dual-reed octave coupler (Bass + Treble reed banks)

public:
    /**
     * @brief Constructs a Harmonium instance.
     * @param musicSystem Shared pointer to the audio system backend.
     * @param enableCoupler Flag to enable octave coupler (default: true).
     *
     * Design Choice: Coupler enabled by default to reflect common harmonium usage in accompaniment.
     */
    explicit Harmonium(std::shared_ptr<IMusicSystem> musicSystem, bool enableCoupler = true);

    /**
     * @brief Default destructor.
     */
    ~Harmonium() override = default;

    // ---------------- IMusicInstrument Overrides ----------------

    /**
     * @brief Retrieves the instrument name.
     * @return Name of the instrument (e.g., "Harmonium").
     */
    std::string GetName() const override;

    /**
     * @brief Plays a single note on the harmonium.
     * @param frequencyHz Frequency of the note in Hertz.
     * @param durationSeconds Duration of the note in seconds (default: 1.5s).
     * @param velocity Intensity of the note (default: 0.8).
     *
     * Physics Note: Frequency maps to pitch, duration to sustain time, and velocity to airflow pressure.
     * Coupler doubles the note an octave apart if enabled.
     */
    void PlayNote(double frequencyHz, double durationSeconds = 1.5, double velocity = 0.8) override;

    /**
     * @brief Plays a chord consisting of multiple frequencies.
     * @param frequencies Vector of frequencies (Hz) representing chord notes.
     * @param durationSeconds Duration of the chord in seconds (default: 2.5s).
     * @param velocity Intensity of the chord (default: 0.8).
     *
     * Design Choice: Chords are modeled as simultaneous reed activation, with coupler enriching harmonic texture.
     */
    void PlayChord(const std::vector<double>& frequencies,
                   double durationSeconds = 2.5,
                   double velocity = 0.8) override;

    // ---------------- Harmonium-Specific Controls ----------------

    /**
     * @brief Enables or disables the octave coupler.
     * @param enabled Boolean flag to set coupler state.
     *
     * Physics Note: Coupler engages dual reed banks, producing octave doubling for richer sound.
     */
    void SetCoupler(bool enabled);

    /**
     * @brief Plays a sustained drone note.
     * @param rootFreqHz Frequency of the root note (Hz).
     * @param durationSeconds Duration of the drone in seconds (default: 4.0s).
     * @param velocity Intensity of the drone (default: 0.7).
     *
     * Design Choice: Drone simulates continuous airflow sustaining a root note, common in Indian classical music.
     */
    void PlayDrone(double rootFreqHz, double durationSeconds = 4.0, double velocity = 0.7);
};