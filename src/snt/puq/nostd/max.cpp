#include <snt/puq/nostd/max.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat max(const puq::MagnitudeFloat& m1, const puq::MagnitudeFloat& m2) {
    return std::max(m1, m2);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2) {
    return a1->math_max(a2.get());
  }
#endif

  extern puq::Magnitude max(const puq::Magnitude& m1, const puq::Magnitude& m2) {
    // x ± Dx = max(x ± Dx, y ± Dy)  <- if x > y
    // y ± Dy = max(x ± Dx, y ± Dy)  <- if y > x
#ifdef MAGNITUDE_VALUES
    MAGNITUDE_VALUE value = m1.value->math_max(m2.value.get());
    if (m1.uncertainty && m2.uncertainty) {
      val::BaseValue::PointerType new_uncertainty = m1.uncertainty->where(value->compare_equal(m1.value.get()).get(), m2.uncertainty.get());
      return puq::Magnitude(std::move(value), std::move(new_uncertainty));
    } else if (m1.uncertainty) {
      return puq::Magnitude(std::move(value), m1.uncertainty->clone());
    } else if (m2.uncertainty) {
      return puq::Magnitude(std::move(value), m2.uncertainty->clone());
    } else {
      return puq::Magnitude(std::move(value));
    }
#else
    MAGNITUDE_VALUE value = max(m1.value, m2.value);
    return puq::Magnitude(value, (value == m1.value) ? m1.uncertainty : m2.uncertainty);
#endif
  }

} // namespace snt::nostd
