#include <snt/puq/nostd/max.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat max(const puq::MagnitudeFloat& m1, const puq::MagnitudeFloat& m2) {
    return std::max(m1, m2);
  }

  val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2) {
    return a1->math_max(a2.get());
  }

  extern puq::Magnitude max(const puq::Magnitude& m1, const puq::Magnitude& m2) {
    // x ± Dx = max(x ± Dx, y ± Dy)  <- if x > y
    // y ± Dy = max(x ± Dx, y ± Dy)  <- if y > x
    val::BaseValue::PointerType estimate = m1.estimate->math_max(m2.estimate.get());
    if (m1.uncertainty && m2.uncertainty) {
      val::BaseValue::PointerType new_uncertainty = m1.uncertainty->where(estimate->compare_equal(m1.estimate.get()).get(), m2.uncertainty.get());
      return puq::Magnitude(std::move(estimate), std::move(new_uncertainty));
    } else if (m1.uncertainty) {
      return puq::Magnitude(std::move(estimate), m1.uncertainty->clone());
    } else if (m2.uncertainty) {
      return puq::Magnitude(std::move(estimate), m2.uncertainty->clone());
    } else {
      return puq::Magnitude(std::move(estimate));
    }
    //val::BaseValue::PointerType estimate = max(m1.estimate, m2.estimate);
    //return puq::Magnitude(estimate, (estimate == m1.estimate) ? m1.uncertainty : m2.uncertainty);
  }

} // namespace snt::nostd
