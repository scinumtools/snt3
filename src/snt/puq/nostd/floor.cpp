#include <snt/puq/nostd/floor.h>

namespace snt::nostd {

  puq::MagnitudeFloat floor(const puq::MagnitudeFloat& m) {
    return std::floor(m);
  }

  val::BaseValue::PointerType floor(val::BaseValue::PointerType a) {
    return a->math_floor();
  }

  puq::Magnitude floor(const puq::Magnitude& m) {
    return puq::Magnitude(m.estimate->math_floor());
    //return puq::Magnitude(floor(m.estimate));
  }

} // namespace snt::nostd
