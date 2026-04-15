#include <snt/puq/math/max.h>

namespace snt::puq::math {

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
    // val::BaseValue::PointerType estimate = max(m1.estimate, m2.estimate);
    // return puq::Result(estimate, (estimate == m1.estimate) ? m1.uncertainty : m2.uncertainty);
  }

  extern puq::Measurement max(const puq::Measurement& msr1, const puq::Measurement& msr2) {
    // x ± Dx = max(x ± Dx, y ± Dy)  <- if x > y
    // y ± Dy = max(x ± Dx, y ± Dy)  <- if y > x
    val::BaseValue::PointerType estimate = msr1.result.estimate->math_max(msr2.result.estimate.get());
    if (msr1.result.uncertainty && msr2.result.uncertainty) {
      val::BaseValue::PointerType new_uncertainty = msr1.result.uncertainty->where(estimate->compare_equal(msr1.result.estimate.get()).get(), msr2.result.uncertainty.get());
      return puq::Measurement(std::move(estimate), std::move(new_uncertainty));
    } else if (msr1.result.uncertainty) {
      return puq::Measurement(std::move(estimate), msr1.result.uncertainty->clone());
    } else if (msr2.result.uncertainty) {
      return puq::Measurement(std::move(estimate), msr2.result.uncertainty->clone());
    } else {
      return puq::Measurement(std::move(estimate));
    }
    // val::BaseValue::PointerType estimate = max(msr1.result.estimate, msr2.result.estimate);
    // return puq::Measurement(estimate, (estimate == msr1.result.estimate) ? msr1.result.uncertainty : msr2.result.uncertainty);
  }

} // namespace snt::puq::math
