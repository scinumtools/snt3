#include <snt/dip/environment.h>
#include <snt/dip/lists/list_unit.h>
#include <snt/puq/systems/unit_system.h>
#include <stdexcept>

namespace snt::dip {

    UnitList::UnitList() = default;

    void UnitList::append(const std::string& name, const std::string& definition) {
        if (units.contains(name))
            throw std::invalid_argument("Custom unit with the same name already exists: " + name);
        size_t stack = puq::UnitSystem::set_custom_unit(name, definition);
        units.insert({name, {name, definition, stack}});
    }

    void UnitList::append(const std::string& name, EnvUnit data) {
        if (units.contains(name))
            throw std::invalid_argument("Custom unit with the same name already exists: " + name);
        data.stack = puq::UnitSystem::set_custom_unit(name, data.definition);
        units.insert({name, data});
    }

    EnvUnit& UnitList::at(const std::string& name) {
        auto it = units.find(name);
        if (it == units.end())
            throw std::runtime_error("Following unit was not found in the environment unit list: " + name);
        else
            return it->second;
    }

    const EnvUnit& UnitList::at(const std::string& name) const {
        auto it = units.find(name);
        if (it == units.end())
            throw std::runtime_error("Following unit was not found in the environment unit list: " + name);
        else
            return it->second;
    }

} // namespace snt::dip
