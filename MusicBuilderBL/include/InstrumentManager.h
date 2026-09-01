#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "IMusicSystem.h"
#include "IMusicInstrument.h"

class InstrumentManager 
{
private:
    std::unordered_map<std::string, std::shared_ptr<IMusicInstrument>> m_instruments;

public:
    explicit InstrumentManager(const std::shared_ptr<IMusicSystem>& audioSystem);

    std::shared_ptr<IMusicInstrument> GetInstrument(const std::string& name) const;
    bool HasInstrument(const std::string& name) const;
};