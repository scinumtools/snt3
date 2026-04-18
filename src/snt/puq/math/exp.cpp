#include <snt/puq/math/exp.h>
#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>
#include <snt/puq/quantity.h>

namespace snt::puq::math {

  puq::Result exp(const puq::Result& res) {
    // z ± Dz = pow(e, y ± Dy) -> Dz = pow(e, y) * log(e) * Dy
    if (res.uncertainty)
      return puq::Result(res.estimate->math_exp(),
			 res.estimate->math_exp()->math_mul(res.uncertainty.get()));
    else
      return puq::Result(res.estimate->math_exp());
    // return puq::Result(exp(res.estimate), exp(res.estimate) * res.uncertainty);
  }

  puq::Measurement exp(const puq::Measurement& msr) {
    return puq::Measurement(exp(msr.result),
			    msr.baseunits);
  }

  puq::Quantity exp(const puq::Quantity& quant) {
    return puq::Quantity(exp(quant.measurement),
			 quant.stype);
  }
  
} // namespace snt::puq::math
