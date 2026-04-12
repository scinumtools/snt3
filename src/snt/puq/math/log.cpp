#include <snt/puq/exponent.h>
#include <snt/puq/math/log.h>

namespace snt::puq::math {

  puq::Result log(const puq::Result& m) {
    // y ± Dy = log10(x ± Dx) -> Dy = Dx / x
    if (m.uncertainty)
      return puq::Result(m.estimate->math_log(), m.uncertainty->math_div(m.estimate.get()));
    else
      return puq::Result(m.estimate->math_log());
    // return puq::Result(log(m.estimate), m.uncertainty / m.estimate);
  }

  puq::Measurement log(const puq::Measurement& msr) {
    // y ± Dy = log10(x ± Dx) -> Dy = Dx / x
    if (msr.magnitude.uncertainty)
      return puq::Measurement(msr.magnitude.estimate->math_log(), msr.magnitude.uncertainty->math_div(msr.magnitude.estimate.get()));
    else
      return puq::Measurement(msr.magnitude.estimate->math_log());
    // return puq::Measurement(log(msr.magnitude.estimate), msr.magnitude.uncertainty / msr.magnitude.estimate);
  }

} // namespace snt::puq::math
