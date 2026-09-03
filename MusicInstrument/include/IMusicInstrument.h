/**
 * @file IMusicInstrument.h
 * @author Soumyajit C, 2026
 * @brief Interface for melodic sound generators in the MusicInstrument module.
 * @brief Interface defining physical modeling and melodic synthesis instruments.
 * @details Establishes the contracts for acoustic simulation models (e.g., Harmonium,
 *          Grand Piano, Kalimba, Strings, and Winds). Enables polyphonic playback,
 *          chord harmonization, and raw sample synthesis decoupled from hardware output.
 */

#pragma once

#include "MusicInstrumentExport.h"

#include <string>
#include <vector>

/**
 * @class IMusicInstrument
 * @brief Interface for all melodic sound generators in the MusicInstrument module.
 */
/*
    Key Architectural Decision: 
    Ensures callers do not unintentionally discard the return value of 
    non-mutating queries (GetName) or buffer generators (SynthesizeNote).
    SynthesizeNote Pure Virtual Method: Bridges the contract with 
    MusicBuilderBL's asynchronous dispatchers 
    (as detailed in your architectural and sequence diagrams), allowing DSP
     generation on background worker threads before handing PCM vectors
    to WindowsMusicSystem::MixAudioAsync.Standardized Doxygen Annotations: 
    Clarifies valid input ranges (velocity $[0.0, 1.0]$, frequencyHz $> 0$) 
    and phase components (ADSR lifecycle).
*/
class MI_API IMusicInstrument 
{
public:
    /**
     * @brief Virtual destructor ensuring safe polymorphic deletion of derived instrument instances.
     */
    virtual ~IMusicInstrument() = default;

    /**
     * @brief Retrieves the human-readable identifier of the instrument.
     * @return Standard string representing the model name (e.g., "Harmonium", "Piano").
     */
    [[nodiscard]] virtual std::string GetName() const = 0;

    /**
     * @brief Synthesizes and plays a single pitch directly through the audio subsystem.
     * @param frequencyHz Fundamental tone frequency in Hertz (must be positive, typically 20.0 to 20000.0).
     * @param durationSeconds Total sounding duration including attack, decay, sustain, and release phases.
     * @param velocity Normalized dynamic volume scalar in the range [0.0, 1.0].
     */
    virtual void PlayNote(double frequencyHz, double durationSeconds = 1.0, double velocity = 0.8) = 0;

    /**
     * @brief Synthesizes and plays multiple simultaneous pitches as a unified harmonic structure.
     * @param frequencies Collection of fundamental frequencies (in Hertz) sounding together.
     * @param durationSeconds Sounding duration applied across all chord constituents.
     * @param velocity Normalized overall volume scalar in the range [0.0, 1.0].
     */
    virtual void PlayChord(const std::vector<double>& frequencies, double durationSeconds = 2.0, double velocity = 0.8) = 0;
};