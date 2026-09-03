/**
 * @file IWindInstrument.h
 * @brief Pure abstract interface defining physical modeling and articulation for wind instruments.
 * @details Establishes the interface for wind and brass acoustic simulation models (e.g., Alto Saxophone,
 *          Bb Trumpet). Provides articulation mechanisms including acoustic mute filtering, rapid tonguing
 *          re-articulation sequences, and decoupled floating-point PCM buffer generation.
 * 
 * @author Soumyajit Chatterjee
 * @copyright © 2026 Soumyajit Chatterjee. All rights reserved.
 */

#pragma once

#include "MusicInstrumentExport.h"

#include <cstdint>
#include <vector>

/**
 * @enum BrassMuteType
 * @brief Acoustic filtering modifiers applied to brass and aerophone acoustic cavities.
 */
enum class BrassMuteType : uint8_t 
{
    None = 0,   ///< Unmuted open acoustic bell radiation.
    Straight,   ///< Bright, piercing attenuation with high-pass spectral tilt.
    Cup,        ///< Muffled, velvety timbre suppressing upper-mid formant peaks.
    Harmon      ///< Resonant, buzzy metallic timbre with tight acoustic cavity notch.
};

/**
 * @class IWindInstrument
 * @brief Interface for physical modeling of aerophones, woodwinds, and brass instruments.
 */
class MI_API IWindInstrument 
{
public:
    /**
     * @brief Virtual destructor ensuring safe polymorphic teardown of derived wind instruments.
     */
    virtual ~IWindInstrument() = default;

    /**
     * @brief Applies an acoustic mute modifier to alter the frequency spectrum and formant response.
     * @param mute Enumerated mute style to configure in the instrument's acoustic wave-guide filter.
     */
    virtual void SetMute(BrassMuteType mute) = 0;

    /**
     * @brief Performs rapid re-articulation and staccato note triggers (single, double, or triple tonguing).
     * @param frequencyHz Fundamental sounding frequency in Hertz.
     * @param noteCount Total number of staccato pulses in the articulation burst.
     * @param noteDuration Duration of each individual note pulse in seconds.
     * @param velocity Normalized air-stream pressure / strike intensity scalar in the range [0.0, 1.0].
     */
    virtual void Tonguing(double frequencyHz, int noteCount, double noteDuration = 0.2, double velocity = 0.8) = 0;
};