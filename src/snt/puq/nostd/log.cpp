#include <snt/puq/nostd/log.h>

#include <snt/puq/exponent.h>

namespace snt::nostd {

  puq::MagnitudeFloat log(const puq::MagnitudeFloat& m) {
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

} // namespace snt::nostd
