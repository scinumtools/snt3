#include <fstream>
#include <snt/puq/exceptions.h>
#include <snt/puq/systems/unit_system.h>
#include <sstream>
#include <sys/stat.h>

namespace snt::puq {

    /**
     * @brief Initialize map of available unit systems
     *
     * @return Map of available unit systems and pointers to their data
     */
    inline std::unordered_map<puq::SystemType, SystemDataType*> get_system_map() {
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

    std::unordered_map<puq::SystemType, SystemDataType*> SystemMap = get_system_map();

    /**
     * @brief Initialize curent unit system record
     *
     * @return Default unit system record
     */
    inline UnitSystem::Record get_default_record() {
        return UnitSystem::Record(SystemType::SI, &SystemData::SI);
    }

    UnitSystem::Record UnitSystem::current = get_default_record();

    std::stack<UnitSystem::Record> UnitSystem::stack;

    /**
     * @brief Find given system type in the system map and return pointer to its data
     *
     * @param system Selected system type
     * @return Pointer to found unit system data
     */
    inline SystemDataType* select_unit_system(const SystemType& system) {
        auto it = SystemMap.find(system);
        if (it == SystemMap.end())
            throw UnitSystemExcept("Unknown system of units: " + std::to_string((int)system));
        return it->second;
    }

    UnitSystem::UnitSystem(const SystemType system) : closed(false) {
        stack.push(current);
        current = Record(system, select_unit_system(system));
    }

    UnitSystem::~UnitSystem() {
        if (!closed)
            close();
    }

    void UnitSystem::change(const SystemType system) {
        current = Record(system, select_unit_system(system));
    }

    void UnitSystem::close() {
        if (closed)
            throw UnitSystemExcept("Instance of a unit system environment cannot be closed only once!");
        current = stack.top();
        stack.pop();
        closed = true;
    }

} // namespace snt::puq
