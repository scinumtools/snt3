#include <snt/puq/math/abs.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

  puq::Result abs(const puq::Result& res) {
    // abs(y ± Dy) = abs(y) ± Dy
    if (res.uncertainty)
      return puq::Result(res.estimate->math_abs(),
                         res.uncertainty->clone());
    else
      return puq::Result(res.estimate->math_abs());
    // return puq::Result(abs(m.estimate), m.uncertainty);
  }

  puq::Measurement abs(const puq::Measurement& msr) {
    return puq::Measurement(abs(msr.result),
                            msr.baseunits);
  }

  puq::Quantity abs(const puq::Quantity& quant) {
    return puq::Quantity(abs(quant.measurement),
                         quant.stype);
  }

} // namespace snt::puq::math
