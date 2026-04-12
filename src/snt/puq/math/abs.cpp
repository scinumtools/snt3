#include <snt/puq/math/abs.h>

namespace snt::puq::math {

  puq::Result abs(const puq::Result& m) {
    // abs(y ± Dy) = abs(y) ± Dy
    return puq::Result(m.estimate->math_abs(), m.uncertainty->clone());
    // return puq::Result(abs(m.estimate), m.uncertainty);
  }

  puq::Measurement abs(const puq::Measurement& msr) {
    // abs(y ± Dy) = abs(y) ± Dy
    return puq::Measurement(msr.magnitude.estimate->math_abs(), msr.magnitude.uncertainty->clone());
    // return puq::Measurement(abs(msr.magnitude.estimate), msr.magnitude.uncertainty);
  }

} // namespace snt::puq::math
