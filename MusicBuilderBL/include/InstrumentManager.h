/**
 * @file InstrumentManager.h
 * @author Soumyajit C
 * @brief Manages musical instruments and provides access by name.
 * @date 2026-09-03
 *
 * The InstrumentManager class maintains a registry of musical instruments
 * associated with a given audio system. It allows retrieval of instruments
 * by name and provides utility functions to check availability.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "IMusicSystem.h"
#include "IMusicInstrument.h"

/**
 * @class InstrumentManager
 * @brief Registry and manager for musical instruments.
 *
 * Responsibilities:
 * - Maintain a mapping of instrument names to instrument instances.
 * - Provide access to instruments by name.
 * - Check whether a given instrument exists.
 *
 * Usage:
 * - Construct with a shared pointer to an IMusicSystem.
 * - Use GetInstrument(name) to retrieve an instrument.
 * - Use HasInstrument(name) to check availability.
 */
class InstrumentManager 
{
private:
    std::unordered_map<std::string, std::shared_ptr<IMusicInstrument>> m_instruments; ///< Instrument registry

public:
    /**
     * @brief Constructs an InstrumentManager bound to an audio system.
     * @param audioSystem [in] Shared pointer to the audio system.
     *
     * Initializes and registers available instruments for the given system.
     */
    explicit InstrumentManager(const std::shared_ptr<IMusicSystem>& audioSystem);

    /**
     * @brief Retrieves an instrument by name.
     * @param name [in] Instrument name.
     * @return Shared pointer to the instrument, or nullptr if not found.
     */
    std::shared_ptr<IMusicInstrument> GetInstrument(const std::string& name) const;

    /**
     * @brief Checks if an instrument exists in the registry.
     * @param name [in] Instrument name.
     * @return true if the instrument exists, false otherwise.
     */
    bool HasInstrument(const std::string& name) const;
};