#include "nostd/nostd.h"
#include "systems/systems.h"
#include "util/data_table.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace snt::puq {

  namespace lists {

    std::string unit_systems(const bool json) {
      DataTable tab({{"Symbol", 6}, {"Name", 30}});
      for (auto sys : SystemMap) {
        tab.append({sys.second->SystemAbbrev, sys.second->SystemName});
      }
      return (json) ? tab.to_json() : tab.to_string();
    }

    std::string prefixes(const bool json, const UnitFormat& format) {
      DataTable tab({{"Symbol", 8}, {"Name", 8}, {"Magnitude", 11}});
      for (auto& symbol : UnitPrefixOrder) {
        auto prefix = UnitPrefixList.at(symbol);
        tab.append({symbol, prefix.name, nostd::to_string(prefix.magnitude, format)});
      }
      return (json) ? tab.to_json() : tab.to_string();
    }

    std::string base_units(const bool json, const UnitFormat& format) {
      DataTable tab({{"Symbol", 8}, {"Name", 19}, {"Allowed prefixes", 22}});
      for (auto& symbol : SystemData::BaseUnitOrder) {
        auto unit = UnitSystem::Data->UnitList.at(symbol);
        if ((unit.utype & Utype::BAS) != Utype::BAS)
          continue;
        tab.append({symbol, unit.name, nostd::to_string(unit.use_prefixes, unit.allowed_prefixes)});
      }
      return (json) ? tab.to_json() : tab.to_string();
    }

    std::string derived_units(const bool json, const UnitFormat& format) {
      DataTable tab({{"Symbol", 9}, {"Name", 22}, {"Magnitude", 13}, {"Dimension", 30}, {"Definition", 25}, {"Allowed prefixes", 22}});
      std::map<std::string, UnitStruct> ordered(UnitSystem::Data->UnitList.begin(), UnitSystem::Data->UnitList.end());
      for (auto& unit : ordered) {
        if ((unit.second.utype & Utype::LIN) != Utype::LIN)
          continue;
        if ((unit.second.utype & Utype::BAS) == Utype::BAS)
          continue;
        if ((unit.second.utype & Utype::CST) == Utype::CST)
          continue;
        if ((unit.second.utype & Utype::CSB) == Utype::CSB)
          continue;
        UnitValue uv(unit.first);
        Dimensions dim = uv.baseunits.dimensions();
        tab.append({unit.first,
                    unit.second.name,
                    dim.to_string(format.merge(Format::Display::MAGNITUDE)),
                    dim.to_string(format.merge(Format::Display::UNITS)),
                    unit.second.definition,
                    nostd::to_string(unit.second.use_prefixes, unit.second.allowed_prefixes)});
      }
      return (json) ? tab.to_json() : tab.to_string();
    }

    std::string constants(const bool json, const UnitFormat& format) {
      DataTable tab({{"Symbol", 9}, {"Name", 19}, {"Magnitude", 30}, {"Dimension", 20}, {"Definition", 30}});
      std::map<std::string, UnitStruct> ordered(UnitSystem::Data->UnitList.begin(), UnitSystem::Data->UnitList.end());
      for (auto& unit : ordered) {
        if ((unit.second.utype & Utype::CST) != Utype::CST)
          continue;
        UnitValue uv(unit.first);
        Dimensions dim = uv.baseunits.dimensions();
        tab.append({unit.first,
                    unit.second.name,
                    dim.to_string(format.merge(Format::Display::MAGNITUDE)),
                    dim.to_string(format.merge(Format::Display::UNITS)),
                    unit.second.definition});
      }
      return (json) ? tab.to_json() : tab.to_string();
    }

    std::string quantities(const bool json, const UnitFormat& format) {
      DataTable tab({{"Symbol", 10}, {"Name", 30}, {"Magnitude", 30}, {"Dimension", 22}, {"Definition", 25}, {"SI factor", 11}});
      std::map<std::string, QuantityStruct> ordered(UnitSystem::Data->QuantityList.begin(), UnitSystem::Data->QuantityList.end());
      for (auto& quantity : ordered) {
        std::string symbol = SYMBOL_QUANTITY_START + quantity.first + SYMBOL_QUANTITY_END;
        UnitValue uv(symbol);
        Dimensions dim = uv.baseunits.dimensions();
        tab.append({symbol,
                    QuantityNames.at(quantity.first),
                    dim.to_string(format.merge(Format::Display::MAGNITUDE)),
                    dim.to_string(format.merge(Format::Display::UNITS)),
                    quantity.second.definition,
                    ((quantity.second.sifactor == "") ? "no" : "yes")});
      }
      return (json) ? tab.to_json() : tab.to_string();
    }

#ifdef UNITS_TEMPERATURES
    std::string temperature_units(const bool json) {
      DataTable tab({{"Symbol", 9}, {"Name", 19}, {"Allowed prefixes", 22}});
      std::map<std::string, UnitStruct> ordered(UnitSystem::Data->UnitList.begin(), UnitSystem::Data->UnitList.end());
      for (auto& unit : ordered) {
        if ((unit.second.utype & Utype::TMP) != Utype::TMP)
          continue;
        tab.append({unit.first, unit.second.name, nostd::to_string(unit.second.use_prefixes, unit.second.allowed_prefixes)});
      }
      return (json) ? tab.to_json() : tab.to_string();
    }
#endif

#ifdef UNITS_LOGARITHMIC
    std::string logarithmic_units(const bool json) {
      DataTable tab({{"Symbol", 9}, {"Name", 19}, {"Allowed prefixes", 22}});
      std::map<std::string, UnitStruct> ordered(UnitSystem::Data->UnitList.begin(), UnitSystem::Data->UnitList.end());
      for (auto& unit : ordered) {
        if ((unit.second.utype & Utype::LOG) != Utype::LOG)
          continue;
        tab.append({unit.first, unit.second.name, nostd::to_string(unit.second.use_prefixes, unit.second.allowed_prefixes)});
      }
      return (json) ? tab.to_json() : tab.to_string();
    }
#endif

  } // namespace lists

} // namespace snt::puq
