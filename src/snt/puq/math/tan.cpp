#include "trigonometric_input.h"

#include <snt/puq/math/tan.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>
#include <snt/puq/systems/unit_system.h>

namespace snt::puq::math {

    puq::Result tan(const puq::Result& m) {
        // y ± Dy = tan(x ± Dx) -> Dy = |sec^2(x)| * Dx = (1 / cos^2(x)) * Dx
        return m.transformed_lazy(m.estimate->math_tan(), [&] {
            auto cosx = m.estimate->math_cos();
            val::ArrayValueFloat64 eps(1e-12);
            auto near_limit = cosx->math_abs()->compare_less(&eps);
            auto derivative = cosx->math_pow(2)->math_inv();
            return derivative->math_inf()->where(near_limit.get(), derivative.get());
        });
    }

    puq::Measurement tan(const puq::Measurement& msr) {
        return puq::Measurement(tan(trigonometric_input(msr, "tangent")));
    }

    puq::Quantity tan(const puq::Quantity& quant) {
        puq::UnitSystem system(quant.stype);
        return puq::Quantity(tan(quant.measurement), quant.stype);
    }

} // namespace snt::puq::math
