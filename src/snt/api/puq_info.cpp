#include "puq_info.h"

#include "snt/puq/to_string.h"

#include <algorithm>
#include <exception>
#include <iomanip>
#include <iostream>
#include <snt/puq/base_units.h>
#include <snt/puq/dimensions.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/util/data_table.h>
#include <sstream>
#include <string>
#include <vector>

namespace snt::api {

    void PUQInfo::argument_input_system(const std::string& system) {
        for (auto sys : puq::SystemMap) {
            if (sys.second->SystemAbbrev == system) {
                input_system = sys.first;
                return;
            }
        }
        throw std::runtime_error("Could not find unit system: " + system);
    }

    std::string PUQInfo::execute() {

        puq::UnitSystem us(puq::SystemType::SI);

        if (input_system != puq::SystemType::NONE) {
            us.change(input_system);
        }

        puq::Quantity q(expression);
        return q.info();
    }

} // namespace snt::api
