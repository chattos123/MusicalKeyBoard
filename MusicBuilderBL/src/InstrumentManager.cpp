/**
 * @file InstrumentManager.cpp
 * @author Soumyajit C
 * @brief Implementation of InstrumentManager for managing musical instruments.
 * @date 2026-09-03
 *
 * The InstrumentManager initializes and registers a set of musical instruments
 * bound to a given audio system. It provides access to instruments by name and
 * supports aliasing for certain instruments (e.g., Saxophone).
 */

#include "InstrumentManager.h"
#include "Piano.h"
#include "Guitar.h"
#include "BassGuitar.h"
#include "Mandolin.h"
#include "Violin.h"
#include "Trumpet.h"
#include "Kalimba.h"
#include "Harmonium.h"
#include "Saxophone.h"

/**
 * @brief Constructs an InstrumentManager and registers available instruments.
 * @param audioSystem [in] Shared pointer to the audio system.
 *
 * Instruments registered:
 * - Piano
 * - Guitar
 * - BassGuitar
 * - Mandolin
 * - Violin
 * - Trumpet
 * - Kalimba
 * - Harmonium (with special flag for drone support)
 * - Saxophone (with aliases "Alto Saxophone" and "AltoSaxophone")
 */
InstrumentManager::InstrumentManager(const std::shared_ptr<IMusicSystem>& audioSystem) 
{
    m_instruments["Piano"]       = std::make_shared<Piano>(audioSystem);
    m_instruments["Guitar"]      = std::make_shared<Guitar>(audioSystem);
    m_instruments["BassGuitar"]  = std::make_shared<BassGuitar>(audioSystem);
    m_instruments["Mandolin"]    = std::make_shared<Mandolin>(audioSystem);
    m_instruments["Violin"]      = std::make_shared<Violin>(audioSystem);
    m_instruments["Trumpet"]     = std::make_shared<Trumpet>(audioSystem);
    m_instruments["Kalimba"]     = std::make_shared<Kalimba>(audioSystem);
    m_instruments["Harmonium"]   = std::make_shared<Harmonium>(audioSystem, true);

    auto sax = std::make_shared<Saxophone>(audioSystem);
    m_instruments["Saxophone"]       = sax;
    m_instruments["Alto Saxophone"]  = sax;
    m_instruments["AltoSaxophone"]   = sax;
}

/**
 * @brief Retrieves an instrument by name.
 * @param name [in] Instrument name.
 * @return Shared pointer to the instrument, or nullptr if not found.
 */
std::shared_ptr<IMusicInstrument> InstrumentManager::GetInstrument(const std::string& name) const 
{
    auto it = m_instruments.find(name);
    return (it != m_instruments.end()) ? it->second : nullptr;
}

/**
 * @brief Checks if an instrument exists in the registry.
 * @param name [in] Instrument name.
 * @return true if the instrument exists, false otherwise.
 */
bool InstrumentManager::HasInstrument(const std::string& name) const 
{
    return m_instruments.find(name) != m_instruments.end();
}