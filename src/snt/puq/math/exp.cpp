#include <snt/puq/math/exp.h>

namespace snt::puq::math {

  puq::Result exp(const puq::Result& e) {
    // z ± Dz = pow(e, y ± Dy) -> Dz = pow(e, y) * log(e) * Dy
    if (e.uncertainty)
      return puq::Result(e.estimate->math_exp(), e.estimate->math_exp()->math_mul(e.uncertainty.get()));
    else
      return puq::Result(e.estimate->math_exp());
    // return puq::Result(exp(e.estimate), exp(e.estimate) * e.uncertainty);
  }

  puq::Measurement exp(const puq::Measurement& msr) {
    return puq::Measurement(exp(msr.result),
			    msr.baseunits);
  }

} // namespace snt::puq::math
