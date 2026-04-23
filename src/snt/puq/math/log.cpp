#include <snt/puq/exponent.h>
#include <snt/puq/math/log.h>
#include <snt/puq/result.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>

namespace snt::puq::math {

  puq::Result log(const puq::Result& m) {
    // y ± Dy = log(x ± Dx) -> Dy = Dx / x
    if (m.uncertainty)
      return puq::Result(m.estimate->math_log(), m.uncertainty->math_div(m.estimate.get()));
    else
      return puq::Result(m.estimate->math_log());
    // return puq::Result(log(m.estimate), m.uncertainty / m.estimate);
  }

  puq::Measurement log(const puq::Measurement& msr) {
    if (msr.baseunits.has_dimensions())
      throw std::runtime_error("Natural logarithm accepts only dimensionless quantities.");
    return puq::Measurement(log(msr.result),
			    msr.baseunits);
  }

  puq::Quantity log(const puq::Quantity& quant) {
    return puq::Quantity(log(quant.measurement),
			 quant.stype);
  }
  
} // namespace snt::puq::math
