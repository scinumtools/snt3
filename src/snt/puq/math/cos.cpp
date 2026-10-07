#include "trigonometric_input.h"

#include <snt/puq/math/cos.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>
#include <snt/puq/systems/unit_system.h>

namespace snt::puq::math {

    puq::Result cos(const puq::Result& m) {
        // y ± Dy = cos(x ± Dx) -> Dy = |sin(x)| * Dx
        if (m.uncertainty)
            return puq::Result(
                m.estimate->math_cos(), m.uncertainty->math_mul(m.estimate->math_sin()->math_abs().get())
            );
        else
            return puq::Result(m.estimate->math_cos());
        // return puq::Result(cos(m.estimate), m.uncertainty / m.estimate);
    }

    puq::Measurement cos(const puq::Measurement& msr) {
        return puq::Measurement(cos(trigonometric_input(msr, "cosine")));
    }

    puq::Quantity cos(const puq::Quantity& quant) {
        puq::UnitSystem system(quant.stype);
        return puq::Quantity(cos(quant.measurement), quant.stype);
    }

} // namespace snt::puq::math
