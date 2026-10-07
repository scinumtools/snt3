#ifndef SNT_PUQ_MATH_TRIGONOMETRIC_INPUT_H
#define SNT_PUQ_MATH_TRIGONOMETRIC_INPUT_H

#include <snt/puq/base_units.h>
#include <snt/puq/exponent.h>
#include <snt/puq/exceptions.h>
#include <snt/puq/measurement.h>
#include <string>

namespace snt::puq::math {

    inline puq::Result trigonometric_input(const puq::Measurement& measurement, const std::string& function) {
        const puq::Dimensions dimensions = measurement.baseunits.dimensions();
        if (!dimensions.has_dimensions())
            return measurement.result;

        const puq::BaseDimensions radians = puq::BaseUnits("rad").dimensions().physical;
        for (int i = 0; i < puq::Config::num_basedim; ++i) {
            if (!puq::equal_exp(dimensions.physical[i], radians[i]))
                throw puq::UnitException(
                    "Dimension mismatch",
                    "The " + function + " function accepts only dimensionless quantities or angles.",
                    "Provide a dimensionless quantity or an angle as the argument of the " + function + " function.",
                    __FILE__,
                    __LINE__
                );
        }
        return measurement.convert("rad").result;
    }

} // namespace snt::puq::math

#endif // SNT_PUQ_MATH_TRIGONOMETRIC_INPUT_H
