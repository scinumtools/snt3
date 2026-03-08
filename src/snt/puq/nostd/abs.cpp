#include <snt/puq/nostd/abs.h>

namespace snt::nostd {

  puq::MagnitudeFloat abs(const puq::MagnitudeFloat& m) {
    return std::abs(m);
  }

  val::BaseValue::PointerType abs(val::BaseValue::PointerType a) {
    return a->math_abs();
  }

  puq::Magnitude abs(const puq::Magnitude& m) {
    // abs(y ± Dy) = abs(y) ± Dy
    return puq::Magnitude(m.estimate->math_abs(), m.uncertainty->clone());
    //return puq::Magnitude(abs(m.estimate), m.uncertainty);
  }

} // namespace snt::nostd
