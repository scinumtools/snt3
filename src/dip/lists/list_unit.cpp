#include <stdexcept>

#include "../environment.h"
#include "lists.h"

namespace dip {

  UnitList::UnitList() {}

  void UnitList::append(const std::string& name, const std::string& definition) {
    if (units.contains(name))
      throw std::invalid_argument("Unit with the same name already exists: " + name);
    units.insert({name, {name, definition}});
  }

  void UnitList::append(const std::string& name, const EnvUnit& src) {
    if (units.contains(name))
      throw std::invalid_argument("Unit with the same name already exists: " + name);
    units.insert({name, src});
  }

  EnvUnit& UnitList::at(const std::string& name) {
    auto it = units.find(name);
    if (it == units.end())
      throw std::runtime_error("Following unit was not found in the environment unit list: " +
                               name);
    else
      return it->second;
  }

  const EnvUnit& UnitList::at(const std::string& name) const {
    auto it = units.find(name);
    if (it == units.end())
      throw std::runtime_error("Following unit was not found in the environment unit list: " +
                               name);
    else
      return it->second;
  }

} // namespace dip
