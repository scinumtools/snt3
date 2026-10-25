#include <snt/puq/base_units.h>
#include <snt/puq/exponent.h>
#include <snt/puq/math/pow.h>
#include <snt/puq/math/sqrt.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

    puq::BaseUnits sqrt(const puq::BaseUnits& bu) {
        return pow(bu, puq::Exponent(1, 2));
    }

    puq::Result sqrt(const puq::Result& res) {
        // y ± Dz = pow(x ± Dx, 0.5) -> Dy = 0.5 * pow(x, -0.5) * Dx
        return res.transformed_lazy(res.estimate->math_sqrt(), [&] {
            auto cst = std::make_unique<val::ArrayValueFloat64>(0.5);
            return res.estimate->math_pow(-0.5)->math_mul(cst.get());
        });
        // return puq::Result(sqrt(res.estimate), 0.5 * pow(res.estimate, -0.5) * res.uncertainty);
    }

    puq::Measurement sqrt(const puq::Measurement& msr) {
        return puq::Measurement(sqrt(msr.result), sqrt(msr.baseunits));
    }

    puq::Quantity sqrt(const puq::Quantity& quant) {
        return puq::Quantity(sqrt(quant.measurement), quant.stype);
    }

} // namespace snt::puq::math
