#include "puq_eval.h"

#include <snt/api/exceptions.h>
#include <snt/puq/calc/calculator.h>

namespace snt::api {

    void PUQEval::argument_input_system(const std::string& system) {
        for (auto sys : puq::SystemMap) {
            if (sys.second->SystemAbbrev == system) {
                input_system = sys.first;
                return;
            }
        }

        std::ostringstream oss;
        for (const auto& sys : puq::SystemMap) {
            if (oss.tellp() > 0)
                oss << ", ";
            oss << "`" << sys.second->SystemAbbrev << "`";
        }
        throw api::ArgumentException(
            "Unknown unit system",
            "The unit system `" + system + "` could not be found.",
            "Use one of the supported unit systems: " + oss.str() + ".",
            __FILE__,
            __LINE__
        );
    }

    void PUQEval::argument_output_system(const std::string& system) {
        for (auto sys : puq::SystemMap) {
            if (sys.second->SystemAbbrev == system) {
                output_system = sys.first;
                return;
            }
        }

        std::ostringstream oss;
        for (const auto& sys : puq::SystemMap) {
            if (oss.tellp() > 0)
                oss << ", ";
            oss << "`" << sys.second->SystemAbbrev << "`";
        }
        throw api::ArgumentException(
            "Unknown unit system",
            "The unit system `" + system + "` could not be found.",
            "Use one of the supported unit systems: " + oss.str() + ".",
            __FILE__,
            __LINE__
        );
    }

    void PUQEval::argument_output_units(const std::string& ounits) {
        output_units = ounits;
    }

    void PUQEval::argument_output_quantity(const std::string& oquantity) {
        output_quantity = oquantity;
    }

    std::string PUQEval::execute() {

        puq::UnitSystem us(puq::SystemType::SI);

        if (input_system != puq::SystemType::NONE) {
            us.change(input_system);
        }

        puq::Calculator calc;
        puq::Quantity q = calc.eval(expression).value;
        if (!output_quantity.empty()) {
            if (output_quantity == "") {
                if (output_system == puq::SystemType::NONE)
                    q = q.convert(output_units);
                else
                    q = q.convert(output_units, output_system);
            } else {
                if (output_system == puq::SystemType::NONE)
                    q = q.convert(output_units, puq::UnitSystem::current.type, output_quantity);
                else {
                    q = q.convert(output_units, output_system, output_quantity);
                }
            }
        }

        return q.to_string();
    }

} // namespace snt::api
