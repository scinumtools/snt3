#include "unit_system.h"

#include "../exceptions.h"

#include <fstream>
#include <sstream>
#include <sys/stat.h>

namespace snt::puq {

  inline std::unordered_map<puq::SystemType, SystemDataType*> GetSystemMap() {
      std::unordered_map<puq::SystemType, SystemDataType*> map = {
	{puq::SystemType::SI, &puq::SystemData::SI},
      };
      if constexpr (Config::use_system_cgs) {
	map.insert({
          {puq::SystemType::ESU, &puq::SystemData::ESU},
          {puq::SystemType::GU, &puq::SystemData::GU},
          {puq::SystemType::EMU, &puq::SystemData::EMU},
	});
      }
      if constexpr (Config::use_system_eus) {
	map.insert({
          {puq::SystemType::IU, &puq::SystemData::IU},
          {puq::SystemType::US, &puq::SystemData::US},
 	});
      }
      if constexpr (Config::use_system_nus) {
	map.insert({
          {puq::SystemType::AU, &puq::SystemData::AU},
          {puq::SystemType::PU, &puq::SystemData::PU},
          {puq::SystemType::SRU, &puq::SystemData::SRU},
          {puq::SystemType::GRU, &puq::SystemData::GRU},
          {puq::SystemType::GEO, &puq::SystemData::GEO},
	});
      }
      return map;
  }
  
  std::unordered_map<puq::SystemType, SystemDataType*> SystemMap = GetSystemMap();

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

} // namespace snt::puq
