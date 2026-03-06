#include <snt/puq/nostd/floor.h>

namespace snt::nostd {

  puq::MagnitudeFloat floor(const puq::MagnitudeFloat& m) {
    return std::floor(m);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType floor(val::BaseValue::PointerType a) {
    return a->math_floor();
  }
#endif

  puq::Magnitude floor(const puq::Magnitude& m) {
#ifdef MAGNITUDE_VALUES
    return puq::Magnitude(m.estimate->math_floor());
#else
    return puq::Magnitude(floor(m.estimate));
#endif
  }

} // namespace snt::nostd
