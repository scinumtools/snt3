#include <snt/puq/math/cbrt.h>
#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>
#include <snt/puq/quantity.h>

namespace snt::puq::math {

  puq::Result cbrt(const puq::Result& res) {
    // y ± Dz = pow(x ± Dx, 1/3) -> Dy = 1/3 * pow(x, -2/3) * Dx
    std::unique_ptr<val::ArrayValue<double>> third1 = std::make_unique<val::ArrayValue<double>>(1. / 3.);
    std::unique_ptr<val::ArrayValue<double>> third2 = std::make_unique<val::ArrayValue<double>>(-2 * 1. / 3.);
    return puq::Result(res.estimate->math_cbrt(), res.estimate->math_pow(third2.get())->math_mul(third1.get())->math_mul(res.uncertainty.get()));
    // constexpr double third = 1. / 3.;
    // return puq::Result(cbrt(res.estimate), third * pow(res.estimate, -2 * third) * res.uncertainty);
  }

  puq::Measurement cbrt(const puq::Measurement& msr) {
    return puq::Measurement(cbrt(msr.result),
			    msr.baseunits);
  }

  puq::Quantity cbrt(const puq::Quantity& quant) {
    return puq::Quantity(cbrt(quant.measurement),
			 quant.stype);
  }
  
} // namespace snt::puq::math
