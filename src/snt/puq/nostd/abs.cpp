#include <snt/puq/nostd/abs.h>

namespace snt::nostd {

  double abs(const double m) {
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

  puq::Measurement abs(const puq::Measurement& msr) {
    // abs(y ± Dy) = abs(y) ± Dy
    return puq::Measurement(msr.magnitude.estimate->math_abs(), msr.magnitude.uncertainty->clone());
    //return puq::Measurement(abs(msr.magnitude.estimate), msr.magnitude.uncertainty);
  }

} // namespace snt::nostd
