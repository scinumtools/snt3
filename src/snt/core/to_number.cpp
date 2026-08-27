#include "to_number.h"

#include <snt/core/exceptions.h>
#include <stdexcept>
#include <typeinfo>

namespace snt::core {

    double to_number(const std::string& value) {
        if (typeid(double) == typeid(float))
            return std::stof(value);
        else if (typeid(double) == typeid(double))
            return std::stod(value);
        else if (typeid(double) == typeid(long double))
            return std::stold(value);
        else
            throw core::ParserException(
                "Unsupported numerical precision",
                "The numerical precision required to convert the value `" + value + "` is not supported.",
                "Use a supported numerical precision for the conversion.",
                __FILE__,
                __LINE__
            );
    }

} // namespace snt::core
