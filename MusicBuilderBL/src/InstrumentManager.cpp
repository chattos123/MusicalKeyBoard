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

InstrumentManager::InstrumentManager(const std::shared_ptr<IMusicSystem>& audioSystem) 
{
    m_instruments["Piano"] = std::make_shared<Piano>(audioSystem);
    m_instruments["Guitar"] = std::make_shared<Guitar>(audioSystem);
    m_instruments["BassGuitar"] = std::make_shared<BassGuitar>(audioSystem);
    m_instruments["Mandolin"] = std::make_shared<Mandolin>(audioSystem);
    m_instruments["Violin"] = std::make_shared<Violin>(audioSystem);
    m_instruments["Trumpet"] = std::make_shared<Trumpet>(audioSystem);
    m_instruments["Kalimba"] = std::make_shared<Kalimba>(audioSystem);
    m_instruments["Harmonium"] = std::make_shared<Harmonium>(audioSystem, true);

    auto sax = std::make_shared<Saxophone>(audioSystem);
    m_instruments["Saxophone"] = sax;
    m_instruments["Alto Saxophone"] = sax;
    m_instruments["AltoSaxophone"] = sax;
}

std::shared_ptr<IMusicInstrument> InstrumentManager::GetInstrument(const std::string& name) const 
{
    auto it = m_instruments.find(name);
    return (it != m_instruments.end()) ? it->second : nullptr;
}

bool InstrumentManager::HasInstrument(const std::string& name) const 
{
    return m_instruments.find(name) != m_instruments.end();
}