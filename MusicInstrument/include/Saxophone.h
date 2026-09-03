/**
 * @file Saxophone.h
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Defines the Saxophone class, a concrete implementation of IMusicInstrument and IWindInstrument.
 *
 * This file declares the Saxophone class, which simulates the acoustic and fluid-dynamic behavior
 * of a conical-bore single-reed aerophone (specifically an Alto Saxophone). It interfaces with the
 * IMusicSystem hardware streaming abstraction to synthesize nonlinear reed excitation, bore
 * wave-guide propagation, radiation impedance filtering, and brass mute modifiers.
 *
 * Design Choices:
 * - Employs dependency injection via std::shared_ptr<IMusicSystem> for decoupled, safe lifetime management.
 * - Implements both IMusicInstrument and IWindInstrument interfaces to support generalized melody/chord
 *   dispatching alongside specialized wind articulation techniques (e.g., staccato tonguing, mutes).
 * - Default parameters for duration and velocity are chosen to mimic realistic woodwind playing styles.
 *
 * Physics Considerations:
 * - The saxophone functions as a closed conical pipe, which produces all natural harmonics (both even
 *   and odd partials), unlike cylindrical single-reed aerophones (e.g., clarinet) that suppress even partials.
 * - Non-linear reed excitation is modeled via differential pressure dynamics modulating instantaneous flow.
 * - Mute modifications simulate physical bell dampers (Straight, Cup, Harmon) altering high-frequency
 *   radiation impedance and formant cavity resonance.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#pragma once

#include "IMusicInstrument.h"
#include "IWindInstrument.h"
#include "IMusicSystem.h"
#include "MusicInstrumentExport.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @class Saxophone
 * @brief Represents an Alto Saxophone instrument with wind-specific articulations.
 *
 * The Saxophone class provides methods to play notes, chords, and perform wind-specific
 * articulations such as acoustic mute filtering and staccato tonguing bursts. It leverages
 * the IMusicSystem backend to synthesize and stream audio output.
 */
class MI_API Saxophone : public IMusicInstrument, public IWindInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_system;              ///< Backend audio system for sound synthesis and hardware playback
    BrassMuteType m_currentMute{ BrassMuteType::None };  ///< Active acoustic mute filter configuration

public:
    /**
     * @brief Constructs an acoustic Saxophone model bound to an audio subsystem.
     * @param system Shared pointer to the audio system backend.
     */
    explicit Saxophone(std::shared_ptr<IMusicSystem> system);

    /**
     * @brief Virtual destructor ensuring safe polymorphic deletion.
     */
    ~Saxophone() override = default;

    // IMusicInstrument Interface

    /**
     * @brief Retrieves the instrument name identifier.
     * @return String literal representing the instrument ("Saxophone").
     */
    std::string GetName() const override;

    /**
     * @brief Synthesizes and plays a single sustained tone through the audio hardware.
     * @param frequency Fundamental tone frequency in Hertz (Alto Sax range: ~138.5 Hz [Db3] to ~880.0 Hz [A5]).
     * @param duration Total sound duration in seconds including breath attack and acoustic decay.
     * @param velocity Normalized embouchure / breath pressure scalar in [0.0, 1.0] (default: 0.8).
     */
    void PlayNote(double frequency, double duration, double velocity = 0.8) override;

    /**
     * @brief Synthesizes and plays a polyphonic wind voicing or multi-reed structure through the audio hardware.
     * @param frequencies Collection of fundamental note frequencies (in Hertz) sounding concurrently.
     * @param duration Total sounding window for the chord in seconds.
     * @param velocity Normalized overall wind pressure scalar across constituent voices (default: 0.8).
     */
    void PlayChord(const std::vector<double>& frequencies, double duration, double velocity = 0.8) override;

    // IWindInstrument Interface

    /**
     * @brief Configures an acoustic mute modifier altering harmonic spectrum and formant filtering.
     * @param mute Enumerated mute type (None, Straight, Cup, Harmon).
     */
    void SetMute(BrassMuteType mute) override;

    /**
     * @brief Executes rapid staccato tonguing pulses on a note through the audio hardware.
     * @param frequencyHz Fundamental pitch frequency in Hertz.
     * @param noteCount Number of distinct staccato pulses in the burst.
     * @param noteDuration Duration of each staccato burst in seconds (default: 0.2s).
     * @param velocity Normalized breath strike velocity in [0.0, 1.0] (default: 0.8).
     */
    void Tonguing(double frequencyHz, int noteCount, double noteDuration = 0.2, double velocity = 0.8) override;
};