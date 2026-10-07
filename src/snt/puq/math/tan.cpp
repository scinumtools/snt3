#include <snt/puq/exceptions.h>
#include <snt/puq/exponent.h>
#include <snt/puq/math/tan.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

    puq::Result tan(const puq::Result& m) {
        // y ± Dy = tan(x ± Dx) -> Dy = |sec^2(x)| * Dx = (1 / cos^2(x)) * Dx
        if (m.uncertainty) {
            // Treating limits where cos(x)->0, or 1/cos2(x)->inf
            val::BaseValue::PointerType cosx = m.estimate->math_cos();
            val::ArrayValueFloat64 eps(1e-12); // defining a small threshold
            auto near_limit = cosx->math_abs()->compare_less(&eps);
            auto invcos2 = cosx->math_pow(2)->math_inv();
            auto propagated = m.uncertainty->math_mul(invcos2.get());
            auto uncertainty = m.uncertainty->math_inf()->where(near_limit.get(), propagated.get());
            return puq::Result(m.estimate->math_tan(), std::move(uncertainty));
        } else {
            return puq::Result(m.estimate->math_tan());
        }
    }

    puq::Measurement tan(const puq::Measurement& msr) {
        if (msr.baseunits.has_dimensions()) // TODO: allow for radians
            throw puq::UnitException(
                "Dimension mismatch",
                "The tangent function accepts only dimensionless quantities.",
                "Provide a dimensionless quantity as the argument of the tangent function.",
                __FILE__,
                __LINE__
            );
        return puq::Measurement(tan(msr.result), msr.baseunits);
    }

    puq::Quantity tan(const puq::Quantity& quant) {
        return puq::Quantity(tan(quant.measurement), quant.stype);
    }

} // namespace snt::puq::math
