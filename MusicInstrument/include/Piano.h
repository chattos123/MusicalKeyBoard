/**
 * @file Piano.h
 * @author Soumyajit Chatterjee
 * @date 03-Sep-2026
 * @brief Defines the Piano class, a concrete implementation of IMusicInstrument modeling an acoustic grand piano.
 *
 * This file declares the Piano class, which implements physical modeling synthesis for an 88-key acoustic
 * grand piano. It interfaces with the IMusicSystem hardware streaming abstraction to synthesize percussive-string
 * excitations, multi-string inharmonicity, soundboard acoustic resonance, and damper decay dynamics.
 *
 * Design Choices:
 * - Employs dependency injection via std::shared_ptr<IMusicSystem> for safe lifetime management and hardware decoupling.
 * - Implements IMusicInstrument to integrate seamlessly with the InstrumentManager factory and dynamic dispatchers.
 * - Provides explicit implementations for single-note synthesis, harmonic chord rendering, and decoupled PCM sample
 *   generation (SynthesizeNote) to support parallel asynchronous background rendering via ThreadPool.
 *
 * Physics & DSP Considerations:
 * - Acoustic piano excitation relies on felt hammer strikes against clamped steel strings.
 * - String stiffness induces slight dispersion and inharmonicity, stretching partial frequencies:
 *   f_n = n * f_0 * sqrt(1 + B * n^2), where B is the inharmonicity coefficient.
 * - Multi-string unisons (duplets in the mid-range, triplets in the treble) exhibit double-decay envelopes:
 *   an initial rapid decay due to coherent string interaction followed by a prolonged, reverberant tail caused
 *   by anti-phase soundboard decoupling.
 * - Velocity directly modulates hammer impact hardness, altering both initial peak amplitude and high-frequency
 *   spectral brightness (dynamic cutoff scaling).
 *
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#pragma once

#include "MusicInstrumentExport.h"
#include "IMusicInstrument.h"
#include "IMusicSystem.h"

#include <memory>
#include <string>
#include <vector>

/**
 * @class Piano
 * @brief Concrete physical-modeling instrument representing an acoustic grand piano.
 *
 * The Piano class simulates struck-string acoustics, sympathetic soundboard resonance,
 * and polyphonic harmonic structures, outputting either directly to the underlying audio
 * system or generating raw IEEE floating-point PCM sample buffers.
 */
class MI_API Piano : public IMusicInstrument
{
private:
    std::shared_ptr<IMusicSystem> m_musicSystem; ///< Audio hardware rendering subsystem
    std::string m_name;                          ///< Human-readable instrument identifier ("Piano")

public:
    /**
     * @brief Constructs an acoustic Piano instance bound to an audio subsystem.
     * @param musicSystem Shared pointer to the initialized audio rendering system.
     */
    explicit Piano(std::shared_ptr<IMusicSystem> musicSystem);

    /**
     * @brief Virtual destructor.
     */
    ~Piano() override = default;

    // -------------------------------------------------------------------------
    // IMusicInstrument Overrides
    // -------------------------------------------------------------------------

    /**
     * @brief Retrieves the instrument name identifier.
     * @return String literal representing the instrument ("Piano").
     */
    [[nodiscard]] std::string GetName() const override;

    /**
     * @brief Synthesizes and plays a single piano note through the audio hardware.
     * @param frequencyHz Fundamental tone frequency in Hertz (typical range: 27.5 Hz [A0] to 4186.0 Hz [C8]).
     * @param durationSeconds Total sounding and decay duration in seconds (default: 1.5s).
     * @param velocity Normalized strike force/gain scalar in the range [0.0, 1.0] (default: 0.8).
     */
    void PlayNote(double frequencyHz, double durationSeconds = 1.5, double velocity = 0.8) override;

    /**
     * @brief Synthesizes and plays a multi-frequency piano chord through the audio hardware.
     * @param frequencies Collection of fundamental note frequencies (in Hertz) composing the chord.
     * @param durationSeconds Total sounding window for the chord in seconds (default: 2.5s).
     * @param velocity Normalized strike force applied across all chord constituent voices (default: 0.8).
     */
    void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.5, double velocity = 0.8) override;
};