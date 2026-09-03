/**
 * @file Violin.h
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Defines the Violin class, a concrete implementation of IMusicInstrument and IStringInstrument.
 *
 * This file declares the Violin class, which models the acoustic and mechanical dynamics of a bowed string
 * instrument (Acoustic Violin). It integrates with the IMusicSystem backend to synthesize physical stick-slip
 * friction oscillations (Helmholtz motion), bridge-to-soundpost transmission, f-hole acoustic radiating cavity
 * formants, pizzicato plucking, multi-string stopped chord strumming, and sustained arco bowing with expressive
 * vibrato modulation.
 *
 * Design Choices:
 * - Employs dependency injection via std::shared_ptr<IMusicSystem> for safe lifetime management and hardware decoupling.
 * - Implements both IMusicInstrument and IStringInstrument interfaces to satisfy general melodic dispatch alongside
 *   string-specific actions (pizzicato plucks, chord strums).
 * - Implements pure virtual SynthesizeNote and SynthesizePluck methods to decouple PCM buffer generation from immediate
 *   audio hardware output, enabling asynchronous multi-threaded rendering via ThreadPool in MusicBuilderBL.
 * - Exposes specialized bowing methods (BowString, SynthesizeBowedNote) providing parametric control over bow pressure,
 *   vibrato pitch deviation depth, and modulation frequency rate.
 *
 * Physics & DSP Considerations:
 * - Standard violin open-string tuning encompasses four fifth-interval courses:
 *   G3 (196.00 Hz), D4 (293.66 Hz), A4 (440.00 Hz), and E5 (659.25 Hz).
 * - Bowed excitation relies on nonlinear friction (stick-slip dynamic), where the string adheres to the rosin-coated
 *   bow hair until restorative elastic tension forces a rapid slip phase, creating Helmholtz sawtooth waves.
 * - Pizzicato plucking switches the excitation mechanism from continuous friction to an impulsive displacement transient
 *   with exponential acoustic body damping.
 * - Vibrato simulates pitch-synchronous low-frequency modulation (LFO) of the fundamental tone (typically 5–7 Hz rate,
 *   3–5 Hz depth), reflecting classical performance technique.
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IStringInstrument.h"
#include "IMusicSystem.h"

#include <memory>
#include <string>
#include <vector>

/**
 * @class Violin
 * @brief Concrete physical-modeling class representing a 4-string acoustic violin.
 *
 * The Violin class synthesizes both bowed (arco) and plucked (pizzicato) string behaviors,
 * outputting directly to the hardware via IMusicSystem or generating raw IEEE 32-bit floating-point
 * PCM buffers for asynchronous mixing.
 */
class MI_API Violin : public IMusicInstrument, public IStringInstrument 
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem; ///< Backend audio system for hardware playback and sample rate queries
    std::string m_name;                          ///< Human-readable instrument identifier ("Violin")
    std::vector<double> m_openTuning;            ///< Fundamental frequencies of open strings: G3 (196.0), D4 (293.66), A4 (440.0), E5 (659.25)

public:
    /**
     * @brief Constructs an acoustic Violin model bound to an audio subsystem.
     * @param musicSystem Shared pointer to the initialized audio rendering system.
     */
    explicit Violin(std::shared_ptr<IMusicSystem> musicSystem);

    /**
     * @brief Virtual destructor.
     */
    ~Violin() override = default;

    // -------------------------------------------------------------------------
    // IMusicInstrument Overrides
    // -------------------------------------------------------------------------

    /**
     * @brief Retrieves the instrument name identifier.
     * @return String literal representing the instrument ("Violin").
     */
    [[nodiscard]] std::string GetName() const override;

    /**
     * @brief Synthesizes and plays a single sustained violin note through the audio hardware.
     * @param frequencyHz Fundamental tone frequency in Hertz (Violin range: ~196.0 Hz [G3] to ~2637.0 Hz [E7]).
     * @param durationSeconds Total sounding duration in seconds (default: 1.5s).
     * @param velocity Normalized bowing pressure/intensity scalar in [0.0, 1.0] (default: 0.8).
     */
    void PlayNote(double frequencyHz, double durationSeconds = 1.5, double velocity = 0.8) override;

    /**
     * @brief Synthesizes and plays multiple simultaneous violin tones as a stopped chord through the audio hardware.
     * @param frequencies Collection of fundamental frequencies (in Hertz) sounding together.
     * @param durationSeconds Sounding window for the chord in seconds (default: 2.0s).
     * @param velocity Normalized overall gain scalar across constituent voices (default: 0.8).
     */
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.0, double velocity = 0.8) override;

    // -------------------------------------------------------------------------
    // IStringInstrument Overrides
    // -------------------------------------------------------------------------

    /**
     * @brief Retrieves the total number of strings configured on the violin.
     * @return Integer count of strings (4 strings: G, D, A, E).
     */
    [[nodiscard]] int GetStringCount() const override;

    /**
     * @brief Plucks a single string (Pizzicato) by index and plays it through the audio hardware.
     * @param stringIndex Zero-based string index (0: G3, 1: D4, 2: A4, 3: E5).
     * @param durationSeconds Total plucked ring and decay duration in seconds (default: 1.0s).
     * @param velocity Normalized strike/pluck intensity in [0.0, 1.0] (default: 0.8).
     */
    void PluckString(int stringIndex, double durationSeconds = 1.0, double velocity = 0.8) override;

    /**
     * @brief Strums a stopped chord across multiple strings with a slight temporal sweep.
     * @param chordFrequencies Frequencies representing the chord voicing.
     * @param strumTimeMs Sweep offset between adjacent string strikes in milliseconds (default: 20.0ms).
     * @param durationSeconds Total decay duration of the strummed voicing in seconds (default: 2.0s).
     */
    void Strum(const std::vector<double>& chordFrequencies, double strumTimeMs = 20.0, double durationSeconds = 2.0) override;


    // -------------------------------------------------------------------------
    // Violin-Specific Performance Techniques (Arco & Vibrato)
    // -------------------------------------------------------------------------

    /**
     * @brief Synthesizes and plays a sustained bowed note with continuous vibrato through the audio hardware.
     * @param frequencyHz Fundamental tone frequency in Hertz.
     * @param durationSeconds Total bow stroke duration in seconds (default: 2.5s).
     * @param bowPressure Normalized bow hair contact pressure in [0.0, 1.0] (default: 0.8).
     * @param vibratoDepthHz Maximum pitch modulation deviation around the fundamental in Hertz (default: 3.5 Hz).
     * @param vibratoRateHz Pitch modulation cycle speed in Hertz (default: 5.5 Hz).
     */
    void BowString(double frequencyHz,
                   double durationSeconds = 2.5,
                   double bowPressure = 0.8,
                   double vibratoDepthHz = 3.5,
                   double vibratoRateHz = 5.5);

    /**
     * @brief Synthesizes raw mono floating-point PCM audio samples for an expressive bowed (arco) note with vibrato.
     * @param frequencyHz Fundamental pitch frequency in Hertz.
     * @param durationSeconds Waveform synthesis duration in seconds.
     * @param bowPressure Normalized bow pressure scalar in [0.0, 1.0].
     * @param vibratoDepthHz Pitch modulation deviation amplitude in Hertz.
     * @param vibratoRateHz Vibrato modulation rate in Hertz.
     * @return Vector of normalized 32-bit floating-point audio samples bounded within [-1.0f, 1.0f].
     */
    [[nodiscard]] std::vector<float> SynthesizeBowedNote(double frequencyHz,
                                                         double durationSeconds,
                                                         double bowPressure,
                                                         double vibratoDepthHz,
                                                         double vibratoRateHz);
};