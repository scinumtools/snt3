#include "floor.h"

namespace snt::nostd {

  MagnitudeFloat floor(const MagnitudeFloat& m) {
    return std::floor(m);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType floor(val::BaseValue::PointerType a) {
    return a->math_floor();
  }
#endif

#ifdef MAGNITUDE_ERRORS
  puq::Magnitude floor(const puq::Magnitude& m) {
#ifdef MAGNITUDE_VALUES
    return puq::Magnitude(m.value->math_floor());
#else
    return puq::Magnitude(floor(m.value));
#endif
  }
#endif

} // namespace snt::nostd
