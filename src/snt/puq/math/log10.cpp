#include <snt/puq/exceptions.h>
#include <snt/puq/math/log10.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

    puq::Result log10(const puq::Result& m) {
        // y ± Dy = log(x ± Dx) -> Dy = 1 / ln(10) * Dx / x
        std::unique_ptr<val::ArrayValueFloat64> cst = std::make_unique<val::ArrayValueFloat64>(std::log(10));
        auto derivative = m.estimate->math_mul(cst.get())->math_inv();
        return m.transformed(m.estimate->math_log10(), std::move(derivative));
        // return puq::Result(log10(m.estimate), m.uncertainty / (m.estimate * std::log(10)));
    }

    puq::Measurement log10(const puq::Measurement& msr) {
        if (msr.baseunits.has_dimensions())
            throw puq::UnitException(
                "Dimension mismatch",
                "The decadic logarithm accepts only dimensionless quantities.",
                "Provide a dimensionless quantity as the argument of the decadic logarithm.",
                __FILE__,
                __LINE__
            );
        return puq::Measurement(log10(msr.result), msr.baseunits);
    }

    puq::Quantity log10(const puq::Quantity& quant) {
        return puq::Quantity(log10(quant.measurement), quant.stype);
    }

} // namespace snt::puq::math
