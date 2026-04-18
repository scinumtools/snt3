#include <snt/puq/math/max.h>
#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>
#include <snt/puq/quantity.h>

namespace snt::puq::math {

  extern puq::Result max(const puq::Result& res1, const puq::Result& res2) {
    // x ± Dx = max(x ± Dx, y ± Dy)  <- if x > y
    // y ± Dy = max(x ± Dx, y ± Dy)  <- if y > x
    // TODO: this implementation is fishy, need a check
    val::BaseValue::PointerType estimate = res1.estimate->math_max(res2.estimate.get());
    if (res1.uncertainty && res2.uncertainty) {
      val::BaseValue::PointerType new_uncertainty = res1.uncertainty->where(estimate->compare_equal(res1.estimate.get()).get(), res2.uncertainty.get());
      return puq::Result(std::move(estimate), std::move(new_uncertainty));
    } else if (res1.uncertainty) {
      return puq::Result(std::move(estimate), res1.uncertainty->clone());
    } else if (res2.uncertainty) {
      return puq::Result(std::move(estimate), res2.uncertainty->clone());
    } else {
      return puq::Result(std::move(estimate));
    }
    // val::BaseValue::PointerType estimate = max(res1.estimate, res2.estimate);
    // return puq::Result(estimate, (estimate == res1.estimate) ? res1.uncertainty : res2.uncertainty);
  }

  extern puq::Measurement max(const puq::Measurement& msr1, const puq::Measurement& msr2) {
    return puq::Measurement(max(msr1.result, msr2.result),
			    msr1.baseunits);
  }

  puq::Quantity max(const puq::Quantity& quant1, const puq::Quantity& quant2) {
    return puq::Quantity(max(quant1.measurement, quant2.measurement),
			 quant1.stype);
  }

} // namespace snt::puq::math
