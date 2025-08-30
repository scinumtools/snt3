#include "../nostd/nostd.h"
#include "systems.h"

#include <fstream>
#include <sstream>
#include <sys/stat.h>

namespace puq {

  std::unordered_map<puq::SystemType, SystemDataType*> SystemMap = {
      {puq::SystemType::SI, &puq::SystemData::SI},
#ifdef UNIT_SYSTEM_CGS
      {puq::SystemType::ESU, &puq::SystemData::ESU},
      {puq::SystemType::GU, &puq::SystemData::GU},
      {puq::SystemType::EMU, &puq::SystemData::EMU},
#endif
#ifdef UNIT_SYSTEM_EUS
      {puq::SystemType::IU, &puq::SystemData::IU},
      {puq::SystemType::US, &puq::SystemData::US},
#endif
#ifdef UNIT_SYSTEM_NUS
      {puq::SystemType::AU, &puq::SystemData::AU},
      {puq::SystemType::PU, &puq::SystemData::PU},
      {puq::SystemType::SRU, &puq::SystemData::SRU},
      {puq::SystemType::GRU, &puq::SystemData::GRU},
      {puq::SystemType::GEO, &puq::SystemData::GEO},
#endif
  };

  SystemDataType* UnitSystem::Data = &SystemData::SI;
  SystemType UnitSystem::System = SystemType::SI;
  std::stack<SystemDataType*> UnitSystem::systemStack;

  inline SystemDataType* select_unit_system(const SystemType& system) {
    auto it = SystemMap.find(system);
    if (it == SystemMap.end())
      throw UnitSystemExcept("Unknown system of units: " + std::to_string((int)system));
    return it->second;
  }

  UnitSystem::UnitSystem(const SystemType system) : closed(false) {
    systemStack.push(Data);
    Data = select_unit_system(system);
    System = system;
  }

  UnitSystem::~UnitSystem() {
    if (!closed)
      close();
  }

  void UnitSystem::change(const SystemType system) {
    Data = select_unit_system(system);
    System = system;
  }

  void UnitSystem::close() {
    if (closed)
      throw UnitSystemExcept("Instance of a unit system environment cannot be closed only once!");
    Data = systemStack.top();
    systemStack.pop();
    bool found = false;
    for (auto sys : SystemMap) {
      if (sys.second == Data) {
        System = sys.first;
        found = true;
      }
    }
    if (!found)
      throw UnitSystemExcept("Could not find previous unit system");
    closed = true;
  }

} // namespace puq
