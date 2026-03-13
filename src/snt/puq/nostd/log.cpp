#include <snt/puq/nostd/log.h>

#include <snt/puq/exponent.h>

namespace snt::nostd {

  double log(const double m) {
    return std::log(m);
  }

  val::BaseValue::PointerType log(val::BaseValue::PointerType a) {
    return a->math_log();
  }

  puq::Magnitude log(const puq::Magnitude& m) {
    // y ± Dy = log10(x ± Dx) -> Dy = Dx / x
    if (m.uncertainty)
      return puq::Magnitude(m.estimate->math_log(), m.uncertainty->math_div(m.estimate.get()));
    else
      return puq::Magnitude(m.estimate->math_log());
    //return puq::Magnitude(log(m.estimate), m.uncertainty / m.estimate);
  }

  puq::Measurement log(const puq::Measurement& msr) {
    // y ± Dy = log10(x ± Dx) -> Dy = Dx / x
    if (msr.magnitude.uncertainty)
      return puq::Measurement(msr.magnitude.estimate->math_log(), msr.magnitude.uncertainty->math_div(msr.magnitude.estimate.get()));
    else
      return puq::Measurement(msr.magnitude.estimate->math_log());
    //return puq::Measurement(log(msr.magnitude.estimate), msr.magnitude.uncertainty / msr.magnitude.estimate);
  }

} // namespace snt::nostd
