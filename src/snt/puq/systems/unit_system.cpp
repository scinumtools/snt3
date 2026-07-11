#include <fstream>
#include <snt/puq/exceptions.h>
#include <snt/puq/quantity.h>
#include <snt/puq/systems/unit_system.h>
#include <sstream>
#include <sys/stat.h>

namespace snt::puq {

    /**
     * @brief Initialize map of available unit systems
     *
     * @return Map of available unit systems and pointers to their data
     */
    inline std::unordered_map<SystemType, SystemDataType*> get_system_map() {
        std::unordered_map<SystemType, SystemDataType*> map = {
            {SystemType::SI, &SystemData::SI},
        };
        if constexpr (Config::use_system_cgs) {
            map.insert({
                {SystemType::ESU, &SystemData::ESU},
                {SystemType::GU, &SystemData::GU},
                {SystemType::EMU, &SystemData::EMU},
            });
        }
        if constexpr (Config::use_system_eus) {
            map.insert({
                {SystemType::IU, &SystemData::IU},
                {SystemType::US, &SystemData::US},
            });
        }
        if constexpr (Config::use_system_nus) {
            map.insert({
                {SystemType::AU, &SystemData::AU},
                {SystemType::PU, &SystemData::PU},
                {SystemType::SRU, &SystemData::SRU},
                {SystemType::GRU, &SystemData::GRU},
                {SystemType::GEO, &SystemData::GEO},
            });
        }
        return map;
    }

    std::unordered_map<SystemType, SystemDataType*> SystemMap = get_system_map();

    /**
     * @brief Initialize map of empty unit systems for custom units
     *
     * @return Map of units systems for custom units
     */
    inline std::unordered_map<SystemType, SystemDataType> get_custom_system_map() {
        std::unordered_map<SystemType, SystemDataType> map;
        for (int i = 0; i < static_cast<int>(SystemType::COUNT); ++i) {
            auto system = static_cast<SystemType>(i);
            map[system] = {};
        }
        return map;
    }

    std::unordered_map<SystemType, SystemDataType> CustomSystemMap = get_custom_system_map();

    /**
     * @brief Initialize curent unit system record
     *
     * @return Default unit system record
     */
    inline UnitSystem::Record get_default_record() {
        return UnitSystem::Record{SystemType::SI, &SystemData::SI, &CustomSystemMap.at(SystemType::SI)};
    }

    UnitSystem::Record UnitSystem::current = get_default_record();

    std::stack<UnitSystem::Record> UnitSystem::stack;

    size_t UnitSystem::set_custom_unit(const std::string& name, const std::string& definition) {
        // Check if custom unit with the same name already exists
        {
            auto it = current.data->UnitList.find(name);
            if (it != current.data->UnitList.end())
                throw UnitSystemExcept("Standard unit with the same name already exist in the current record: " + name);
        }
        {
            auto it = current.custom->UnitList.find(name);
            if (it != current.custom->UnitList.end())
                throw UnitSystemExcept("Custom unit with the same name already exist in the current record: " + name);
        }
        // Extract DimensionStruct values from a quantity
        Quantity quant(definition);
        Result res = quant.measurement.result * quant.measurement.baseunits.dimensions().numerical;
        auto* estimate = dynamic_cast<val::BaseArrayValue<double>*>(res.estimate.get());
        auto* uncertainty = dynamic_cast<val::BaseArrayValue<double>*>(res.uncertainty.get());
        if (!estimate)
            throw std::runtime_error("Custom quantity estimate has invalid type, or is undefined.");
        // Add custom units into UnitList and DimensionMap
        current.custom->UnitList[name] = UnitStruct{UT_LIN_CUS, definition, name, false};
        current.custom->DimensionMap[name] = DimensionStruct{
            estimate->get_value(0),
            (uncertainty ? uncertainty->get_value(0) : 0.),
            quant.measurement.baseunits.dimensions().physical
        };
        return stack.size();
    }

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
        current = Record{system, select_unit_system(system), &CustomSystemMap.at(system)};
    }

    UnitSystem::~UnitSystem() {
        if (!closed)
            close();
    }

    void UnitSystem::change(const SystemType system) {
        current = Record{system, select_unit_system(system), &CustomSystemMap.at(system)};
    }

    void UnitSystem::close() {
        if (closed)
            throw UnitSystemExcept("Instance of a unit system environment cannot be closed only once!");
        current = stack.top();
        stack.pop();
        closed = true;
    }

    const std::string UnitSystem::to_string() const {
        return "UnitSystem(" + current.data->SystemAbbrev + ")";
    }

} // namespace snt::puq
