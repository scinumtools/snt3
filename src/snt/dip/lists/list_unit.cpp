#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_unit.h>
#include <snt/puq/systems/unit_system.h>
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace snt::dip {

    namespace {

        std::string next_unit_id(std::map<std::string, size_t>& counters, const std::string& parent_id) {
            const std::string scope = parent_id.empty() ? "ENV" : parent_id;
            return scope + "_UNIT" + std::to_string(counters[scope]++);
        }

    } // namespace

    UnitList::UnitList() = default;

    void UnitList::append(const std::string& name, const std::string& definition, const std::string& parent_id) {
        append(name, EnvUnit{name, definition}, parent_id);
    }

    void UnitList::append(const std::string& name, EnvUnit data, const std::string& parent_id) {
        auto it = units.find(name);
        if (it != units.end())
            throw dip::EnvironmentException(
                "Duplicate custom unit",
                "A custom unit named `" + name + "` already exists in the environment unit list.",
                "Choose a different custom unit name.",
                __FILE__,
                __LINE__
            );
        const auto active = puq::UnitSystem::current.custom->UnitList.find(name);
        if (active != puq::UnitSystem::current.custom->UnitList.end()) {
            if (active->second.definition != data.definition)
                throw dip::EnvironmentException(
                    "Conflicting custom unit",
                    "The active PUEL unit `" + name + "` has a different definition.",
                    "Load environments with distinct unit names or matching unit definitions.",
                    __FILE__,
                    __LINE__
                );
            data.stack = 0;
        } else {
            data.stack = puq::UnitSystem::set_custom_unit(name, data.definition);
        }
        if (data.id.empty())
            data.id = next_unit_id(id_counters, parent_id);
        if (data.registration_order == std::numeric_limits<size_t>::max())
            data.registration_order = next_registration_order++;
        else
            next_registration_order = std::max(next_registration_order, data.registration_order + 1);
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

    const std::map<std::string, EnvUnit>& UnitList::entries() const {
        return units;
    }

} // namespace snt::dip
