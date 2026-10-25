#include "trigonometric_input.h"

#include <snt/puq/math/sin.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>
#include <snt/puq/systems/unit_system.h>

namespace snt::puq::math {

    puq::Result sin(const puq::Result& m) {
        // y ± Dy = sin(x ± Dx) -> Dy = |cos(x)| * Dx
        return m.transformed(m.estimate->math_sin(), m.estimate->math_cos());
        // return puq::Result(sin(m.estimate), m.uncertainty / m.estimate);
    }

    puq::Measurement sin(const puq::Measurement& msr) {
        return puq::Measurement(sin(trigonometric_input(msr, "sine")));
    }

    puq::Quantity sin(const puq::Quantity& quant) {
        puq::UnitSystem system(quant.stype);
        return puq::Quantity(sin(quant.measurement), quant.stype);
    }

} // namespace snt::puq::math
