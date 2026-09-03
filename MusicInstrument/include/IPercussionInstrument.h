/**
 * @file    IPercussionInstrument.h
 * @author  Soumyajit C, 2026
 * @brief   Interface defining physical modeling and percussion voice synthesis.
 * @details Declares drum piece enumerations and execution contracts for physical drum 
 *          synthesis engines (e.g., DrumKit, Tabla, Western Acoustic Kit). Provides 
 *          direct audio triggering, composite polyphonic beat rendering, piece queries, 
 *          and raw sample buffer synthesis decoupled from hardware output.
 */

#pragma once

#include "MusicInstrumentExport.h"

#include <cstdint>
#include <string>
#include <vector>

/**
 * @enum DrumPiece
 * @brief Identifiers for standard physical membrane, metallic, and noise drum components.
 */
enum class DrumPiece : uint8_t 
{
    BassDrum = 0,   ///< Kick drum (punchy low pitch-sweep sine core).
    SnareDrum,      ///< Snare drum (resonant body tone + filtered noise burst).
    ClosedHiHat,    ///< Metallic short-decay banded noise.
    OpenHiHat,      ///< Metallic sustained-decay banded noise.
    LowTom,         ///< Resonant mid-low membrane with exponential envelope.
    HighTom,        ///< High-frequency resonant membrane.
    CrashCymbal     ///< Broad-spectrum high-density exponential decay wash.
};

/**
 * @class IPercussionInstrument
 * @brief Interface for all rhythm and percussion sound generators.
 */
class MI_API IPercussionInstrument 
{
public:
    /**
     * @brief Virtual destructor ensuring safe polymorphic deletion of derived percussion kits.
     */
    virtual ~IPercussionInstrument() = default;

    /**
     * @brief Synthesizes and plays a single drum voice directly through the active audio subsystem.
     * @param piece Enumerated drum component to strike.
     * @param velocity Normalized strike force/gain scalar in the range [0.0, 1.0].
     * @param durationSeconds Total sound lifetime including impact and resonance decay.
     */
    virtual void HitDrum(DrumPiece piece, double velocity = 0.8, double durationSeconds = 1.0) = 0;

    /**
     * @brief Triggers multiple simultaneous drum voices as a composite polyphonic hit (e.g., Kick + Hat).
     * @param pieces Collection of drum components struck simultaneously.
     * @param durationSeconds Sounding window applied to the composite voice render.
     * @param velocity Normalized strike gain scalar in the range [0.0, 1.0].
     */
    virtual void PlayBeat(const std::vector<DrumPiece>& pieces, double durationSeconds = 1.0, double velocity = 0.8) = 0;

    /**
     * @brief Retrieves descriptive display names for all supported drum pieces in the kit.
     * @return List of strings representing piece names corresponding to the drum kit topology.
     */
    [[nodiscard]] virtual std::vector<std::string> GetDrumPieces() const = 0;

};