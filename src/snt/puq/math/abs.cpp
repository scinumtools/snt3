#include <snt/puq/math/abs.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

    puq::Result abs(const puq::Result& res) {
        // abs(y ± Dy) = abs(y) ± Dy
        auto derivative = res.estimate->math_div(res.estimate->math_abs().get());
        val::ArrayValueFloat64 zero(0.0);
        val::ArrayValueFloat64 one(1.0);
        auto at_zero = res.estimate->compare_equal(&zero);
        derivative = one.where(at_zero.get(), derivative.get());
        return res.transformed(res.estimate->math_abs(), std::move(derivative));
        // return puq::Result(abs(m.estimate), m.uncertainty);
    }

    puq::Measurement abs(const puq::Measurement& msr) {
        return puq::Measurement(abs(msr.result), msr.baseunits);
    }

    puq::Quantity abs(const puq::Quantity& quant) {
        return puq::Quantity(abs(quant.measurement), quant.stype);
    }

} // namespace snt::puq::math
