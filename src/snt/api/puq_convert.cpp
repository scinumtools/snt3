#include "puq_convert.h"

#include <snt/puq/quantity.h>

namespace snt::api {

    void PUQConvert::argument_input_system(const std::string& system) {
        for (auto sys : puq::SystemMap) {
            if (sys.second->SystemAbbrev == system) {
                input_system = sys.first;
                return;
            }
        }
        throw std::runtime_error("Could not find unit system: " + system);
    }

    void PUQConvert::argument_output_system(const std::string& system) {
        for (auto sys : puq::SystemMap) {
            if (sys.second->SystemAbbrev == system) {
                output_system = sys.first;
                return;
            }
        }
        throw std::runtime_error("Could not find unit system: " + system);
    }

    void PUQConvert::argument_output_quantity(const std::string& quantity) {
        output_quantity = quantity;
    }

    std::string PUQConvert::execute() {

        puq::UnitSystem us(puq::SystemType::SI);

        if (input_system != puq::SystemType::NONE) {
            us.change(input_system);
        }

        puq::Quantity q(expression);
        if (output_quantity.empty()) {
            if (output_system == puq::SystemType::NONE) {
                q = q.convert(output_units);
            } else {
                q = q.convert(output_units, output_system);
            }
        } else {
            if (output_system == puq::SystemType::NONE) {
                q = q.convert(output_units, puq::UnitSystem::current.type, output_quantity);
            } else {
                q = q.convert(output_units, output_system, output_quantity);
            }
        }

        return q.to_string();
    }

} // namespace snt::api
