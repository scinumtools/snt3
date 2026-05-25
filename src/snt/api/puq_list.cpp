#include "puq_list.h"

#include <snt/puq/lists.h>
#include <snt/puq/systems/unit_system.h>

namespace snt::api {

    void PUQList::argument_system(const std::string& system) {
        for (auto sys : puq::SystemMap) {
            if (sys.second->SystemAbbrev == system) {
                input_system = sys.first;
                return;
            }
        }
        throw std::runtime_error("Could not find unit system: " + system);
    }

    void PUQList::execute() {

        puq::UnitSystem us(puq::SystemType::SI);

        if (input_system != puq::SystemType::NONE) {
            us.change(input_system);
        }

        if (list == "log")
            std::cout << puq::lists::logarithmic_units();
        else if (list == "temp")
            std::cout << puq::lists::temperature_units();
        else if (list == "prefix")
            std::cout << puq::lists::prefixes();
        else if (list == "base")
            std::cout << puq::lists::base_units();
        else if (list == "deriv")
            std::cout << puq::lists::derived_units();
        else if (list == "const")
            std::cout << puq::lists::constants();
        else if (list == "quant")
            std::cout << puq::lists::quantities();
        else if (list == "sys")
            std::cout << puq::lists::unit_systems();
        else {
            std::stringstream ss;
            ss << '\n';
            if (list.size() > 0)
                ss << "List '" << list << "' does not exist." << '\n'
                   << '\n';
            ss << "Available lists:" << '\n'
               << '\n';
            ss << "prefix  unit prefixes" << '\n';
            ss << "base    base units" << '\n';
            ss << "deriv   derived units" << '\n';
            ss << "log     logarithmic units" << '\n';
            ss << "temp    temperature units" << '\n';
            ss << "const   constants" << '\n';
            ss << "quant   quantities" << '\n';
            ss << "sys     unit systems" << '\n';
            std::cout << ss.str() << '\n';
        }
    }

} // namespace snt::api
