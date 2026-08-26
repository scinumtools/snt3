#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_unit.h>
#include <snt/puq/systems/unit_system.h>
#include <stdexcept>

namespace snt::dip {

    UnitList::UnitList() = default;

    void UnitList::append(const std::string& name, const std::string& definition) {
        auto it = units.find(name);
        if (it != units.end())
            throw dip::EnvironmentException(
                "Duplicate custom unit",
                "A custom unit named `" + name + "` already exists in the environment unit list.",
                "Choose a different custom unit name.",
                __FILE__,
                __LINE__
            );
        size_t stack = puq::UnitSystem::set_custom_unit(name, definition);
        units.insert({name, {name, definition, stack}});
    }

    void UnitList::append(const std::string& name, EnvUnit data) {
        auto it = units.find(name);
        if (it != units.end())
            throw dip::EnvironmentException(
                "Duplicate custom unit",
                "A custom unit named `" + name + "` already exists in the environment unit list.",
                "Choose a different custom unit name.",
                __FILE__,
                __LINE__
            );
        data.stack = puq::UnitSystem::set_custom_unit(name, data.definition);
        units.insert({name, data});
    }

    EnvUnit& UnitList::at(const std::string& name) {
        auto it = units.find(name);
        if (it == units.end())
            throw dip::EnvironmentException(
                "Unknown unit",
                "The unit `" + name + "` was not found in the environment unit list.",
                "Check whether the unit name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

    const EnvUnit& UnitList::at(const std::string& name) const {
        auto it = units.find(name);
        if (it == units.end())
            throw dip::EnvironmentException(
                "Unknown unit",
                "The unit `" + name + "` was not found in the environment unit list.",
                "Check whether the unit name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

} // namespace snt::dip
