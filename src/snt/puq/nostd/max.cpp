#include "max.h"

namespace snt::nostd {

  extern MagnitudeFloat max(const MagnitudeFloat& m1, const MagnitudeFloat& m2) {
    return std::max(m1, m2);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2) {
    return a1->math_max(a2.get());
  }
#endif

#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude max(const puq::Magnitude& m1, const puq::Magnitude& m2) {
    // x ± Dx = max(x ± Dx, y ± Dy)  <- if x > y
    // y ± Dy = max(x ± Dx, y ± Dy)  <- if y > x
#ifdef MAGNITUDE_VALUES
    MAGNITUDE_VALUE value = m1.value->math_max(m2.value.get());
    if (m1.error && m2.error) {
      val::BaseValue::PointerType new_error = m1.error->where(value->compare_equal(m1.value.get()).get(), m2.error.get());
      return puq::Magnitude(std::move(value), std::move(new_error));
    } else if (m1.error) {
      return puq::Magnitude(std::move(value), m1.error->clone());
    } else if (m2.error) {
      return puq::Magnitude(std::move(value), m2.error->clone());
    } else {
      return puq::Magnitude(std::move(value));
    }
#else
    MAGNITUDE_VALUE value = max(m1.value, m2.value);
    return puq::Magnitude(value, (value == m1.value) ? m1.error : m2.error);
#endif
  }
#endif

} // namespace snt::nostd
