/**
 * @file IStringInstrument.h
 * @brief Pure abstract interface defining physical modeling and plucked/strummed chord synthesis.
 * @details Declares contracts for physical string instruments (e.g., Acoustic Guitar,
 *          Electric Bass, Mandolin). Provides string count inspection, discrete single-string
 *          plucking, staggered strum simulations across chord voicings, and raw PCM buffer
 *          synthesis decoupled from hardware playback.
 * 
 * @author Soumyajit C, 2026
 */

#pragma once

#include "MusicInstrumentExport.h"

#include <vector>

/**
 * @class IStringInstrument
 * @brief Interface for all string-based acoustic and electric instrument models.
 */
class MI_API IStringInstrument 
{
public:
    /**
     * @brief Virtual destructor ensuring safe polymorphic deletion of derived string instrument instances.
     */
    virtual ~IStringInstrument() = default;

    /**
     * @brief Retrieves the total number of physical strings configured on the instrument.
     * @return Positive integer representing string count (e.g., 4 for Bass, 6 for Guitar, 8 for Mandolin).
     */
    [[nodiscard]] virtual int GetStringCount() const = 0;

    /**
     * @brief Simulates plucking a single string identified by its index.
     * @param stringIndex Zero-based index of the target string (must be in range [0, GetStringCount() - 1]).
     * @param durationSeconds Total sound lifetime including initial transient pluck and decay resonance.
     * @param velocity Normalized strike force/gain scalar in the range [0.0, 1.0].
     */
    virtual void PluckString(int stringIndex, double durationSeconds = 1.5, double velocity = 0.8) = 0;

    /**
     * @brief Simulates strumming a multi-frequency chord across strings with a staggered temporal offset.
     * @param chordFrequencies Collection of fundamental frequencies (in Hertz) composing the chord.
     * @param strumTimeMs Total roll time in milliseconds taken to sweep across all chord strings.
     * @param durationSeconds Overall decay/ring duration of the strummed voicing in seconds.
     */
    virtual void Strum(const std::vector<double>& chordFrequencies, double strumTimeMs = 25.0, double durationSeconds = 2.5) = 0;
};