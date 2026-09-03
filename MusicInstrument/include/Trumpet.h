/**
 * @file Trumpet.h
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Defines the Trumpet class, a concrete implementation of IMusicInstrument and IWindInstrument.
 *
 * This file declares the Trumpet class, which models the acoustic dynamics and lip-reed mechanics
 * of a standard Bb brass trumpet. It interfaces with an IMusicSystem backend to synthesize lip-buzz
 * oscillations, flared bell radiation impedance, acoustic mute modifications (Straight and Harmon),
 * and specialized brass performance articulations including rapid staccato tonguing and fanfares.
 *
 * Design Choices:
 * - Employs dependency injection via std::shared_ptr<IMusicSystem> for safe memory management and hardware decoupling.
 * - Implements both IMusicInstrument and IWindInstrument interfaces to satisfy polymorphic playback alongside
 *   specialized aerophone/brass acoustic modifiers.
 * - Default parameters for note durations and velocities are tuned to mimic natural brass embouchure dynamics.
 *
 * Physics & Acoustics Notes:
 * - Lip-reed mechanics operate as outward-striking pressure-controlled valves where blowing pressure modulates
 *   both acoustic volume and harmonic spectral bandwidth (higher velocities induce rapid harmonic proliferation).
 * - Mute insertions introduce acoustic cavity filters and zeroes that modify bell resonance and directional radiation.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IWindInstrument.h"
#include "IMusicSystem.h"
#include <string>
#include <vector>
#include <memory>

/**
 * @class Trumpet
 * @brief Represents a Bb brass trumpet with specialized wind articulation and acoustic muting capabilities.
 *
 * The Trumpet class provides methods for single-note synthesis, polyphonic brass section voicings,
 * acoustic mute filtering, rapid tonguing re-articulations, and sequenced fanfare passages.
 */
class MI_API Trumpet : public IMusicInstrument, public IWindInstrument {
private:
    std::shared_ptr<IMusicSystem> m_musicSystem; ///< Backend audio system for sound synthesis
    std::string m_name;                          ///< Instrument name identifier
    BrassMuteType m_mute;                        ///< Active acoustic mute filter configuration

public:
    /**
     * @brief Constructs a Trumpet instance bound to an audio subsystem.
     * @param musicSystem Shared pointer to the audio system backend for sound rendering.
     */
    explicit Trumpet(std::shared_ptr<IMusicSystem> musicSystem);

    /**
     * @brief Default virtual destructor.
     */
    ~Trumpet() override = default;

    // -------------------------------------------------------------------------
    // IMusicInstrument overrides
    // -------------------------------------------------------------------------

    /**
     * @brief Retrieves the human-readable identifier of the instrument.
     * @return Standard string containing the instrument name ("Bb Brass Trumpet").
     */
    std::string GetName() const override;

    /**
     * @brief Synthesizes and plays a single brass note through the audio subsystem.
     * @param frequencyHz Fundamental tone frequency in Hertz.
     * @param durationSeconds Total sound duration in seconds (default: 1.2s).
     * @param velocity Blowing pressure and attack intensity scalar in [0.0, 1.0] (default: 0.8).
     */
    void PlayNote(double frequencyHz, double durationSeconds = 1.2, double velocity = 0.8) override;

    /**
     * @brief Plays multiple brass notes simultaneously as an ensemble chord.
     * @param frequencies Collection of fundamental frequencies in Hertz composing the chord.
     * @param durationSeconds Sounding window duration in seconds (default: 2.0s).
     * @param velocity Blowing intensity scalar across voices in [0.0, 1.0] (default: 0.8).
     */
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.0, double velocity = 0.8) override;

    // -------------------------------------------------------------------------
    // IWindInstrument overrides
    // -------------------------------------------------------------------------

    /**
     * @brief Configures the active acoustic mute modifier on the trumpet.
     * @param mute The BrassMuteType modifier to apply (None, Straight, Harmon).
     */
    void SetMute(BrassMuteType mute) override;

    /**
     * @brief Performs rapid staccato tonguing articulations on a single note.
     * @param frequencyHz Fundamental frequency in Hertz.
     * @param noteCount Total number of staccato pulses in the sequence.
     * @param noteDuration Duration of each individual note pulse in seconds (default: 0.2s).
     * @param velocity Strike intensity scalar in the range [0.0, 1.0] (default: 0.8).
     */
    void Tonguing(double frequencyHz, int noteCount, double noteDuration = 0.2, double velocity = 0.8) override;

    // -------------------------------------------------------------------------
    // Brass-specific fanfare articulation
    // -------------------------------------------------------------------------

    /**
     * @brief Plays a sequenced brass fanfare passage at a given tempo.
     * @param notes Ordered vector of fundamental frequencies in Hertz representing the fanfare melody.
     * @param tempoBpm Tempo in beats per minute governing note durations (default: 120.0 BPM).
     */
    void PlayFanfare(const std::vector<double>& notes, double tempoBpm = 120.0);
};