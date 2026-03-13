#include <snt/puq/nostd/max.h>

namespace snt::nostd {

  extern double max(const double m1, const double m2) {
    return std::max(m1, m2);
  }

  val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2) {
    return a1->math_max(a2.get());
  }

  extern puq::Result max(const puq::Result& m1, const puq::Result& m2) {
    // x ± Dx = max(x ± Dx, y ± Dy)  <- if x > y
    // y ± Dy = max(x ± Dx, y ± Dy)  <- if y > x
    val::BaseValue::PointerType estimate = m1.estimate->math_max(m2.estimate.get());
    if (m1.uncertainty && m2.uncertainty) {
      val::BaseValue::PointerType new_uncertainty = m1.uncertainty->where(estimate->compare_equal(m1.estimate.get()).get(), m2.uncertainty.get());
      return puq::Result(std::move(estimate), std::move(new_uncertainty));
    } else if (m1.uncertainty) {
      return puq::Result(std::move(estimate), m1.uncertainty->clone());
    } else if (m2.uncertainty) {
      return puq::Result(std::move(estimate), m2.uncertainty->clone());
    } else {
      return puq::Result(std::move(estimate));
    }
    //val::BaseValue::PointerType estimate = max(m1.estimate, m2.estimate);
    //return puq::Result(estimate, (estimate == m1.estimate) ? m1.uncertainty : m2.uncertainty);
  }

  extern puq::Measurement max(const puq::Measurement& msr1, const puq::Measurement& msr2) {
    // x ± Dx = max(x ± Dx, y ± Dy)  <- if x > y
    // y ± Dy = max(x ± Dx, y ± Dy)  <- if y > x
    val::BaseValue::PointerType estimate = msr1.magnitude.estimate->math_max(msr2.magnitude.estimate.get());
    if (msr1.magnitude.uncertainty && msr2.magnitude.uncertainty) {
      val::BaseValue::PointerType new_uncertainty = msr1.magnitude.uncertainty->where(estimate->compare_equal(msr1.magnitude.estimate.get()).get(), msr2.magnitude.uncertainty.get());
      return puq::Measurement(std::move(estimate), std::move(new_uncertainty));
    } else if (msr1.magnitude.uncertainty) {
      return puq::Measurement(std::move(estimate), msr1.magnitude.uncertainty->clone());
    } else if (msr2.magnitude.uncertainty) {
      return puq::Measurement(std::move(estimate), msr2.magnitude.uncertainty->clone());
    } else {
      return puq::Measurement(std::move(estimate));
    }
    //val::BaseValue::PointerType estimate = max(msr1.magnitude.estimate, msr2.magnitude.estimate);
    //return puq::Measurement(estimate, (estimate == msr1.magnitude.estimate) ? msr1.magnitude.uncertainty : msr2.magnitude.uncertainty);
  }

} // namespace snt::nostd
