#ifndef PUQ_LISTS_H
#define PUQ_LISTS_H

#include "systems/systems.h"

#include <string>
#include <vector>

namespace puq {

  std::string table_header(std::string title, std::vector<std::string> header, std::vector<int> width);

  namespace lists {
    std::string unit_systems(const bool json = false);
    std::string prefixes(const bool json = false, const UnitFormat& format = UnitFormat());
    std::string base_units(const bool json = false, const UnitFormat& format = UnitFormat());
    std::string derived_units(const bool json = false, const UnitFormat& format = UnitFormat());
    std::string constants(const bool json = false, const UnitFormat& format = UnitFormat());
    std::string quantities(const bool json = false, const UnitFormat& format = UnitFormat());
#ifdef UNITS_TEMPERATURES
    std::string temperature_units(const bool json = false);
#endif
#ifdef UNITS_LOGARITHMIC
    std::string logarithmic_units(const bool json = false);
#endif
  } // namespace lists

} // namespace puq

#endif // PUQ_LISTS_H
