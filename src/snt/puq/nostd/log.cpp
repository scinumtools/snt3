#include "log.h"

#include "../exponent.h"

namespace snt::nostd {

  MAGNITUDE_PRECISION log(const MAGNITUDE_PRECISION& m) {
    return std::log(m);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType log(val::BaseValue::PointerType a) {
    return a->math_log();
  }
#endif

#ifdef MAGNITUDE_ERRORS
  puq::Magnitude log(const puq::Magnitude& m) {
    // y ± Dy = log10(x ± Dx) -> Dy = Dx / x
#ifdef MAGNITUDE_VALUES
    if (m.error)
      return puq::Magnitude(m.value->math_log(), m.error->math_div(m.value.get()));
    else
      return puq::Magnitude(m.value->math_log());
#else
    return puq::Magnitude(log(m.value), m.error / m.value);
#endif
  }
#endif

} // namespace snt::nostd
