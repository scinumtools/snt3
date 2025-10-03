#include "abs.h"

namespace snt::nostd {

  MAGNITUDE_PRECISION abs(const MAGNITUDE_PRECISION& m) {
    return std::abs(m);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType abs(val::BaseValue::PointerType a) {
    return a->math_abs();
  }
#endif

#ifdef MAGNITUDE_ERRORS
  puq::Magnitude abs(const puq::Magnitude& m) {
    // abs(y ± Dy) = abs(y) ± Dy
#ifdef MAGNITUDE_VALUES
    return puq::Magnitude(m.value->math_abs(), m.error->clone());
#else
    return puq::Magnitude(abs(m.value), m.error);
#endif
  }
#endif

} // namespace snt::nostd
