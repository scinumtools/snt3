#include <snt/puq/math/min.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

  extern puq::Result min(const puq::Result& res1, const puq::Result& res2) {
    // x ± Dx = min(x ± Dx, y ± Dy)  <- if x < y
    // y ± Dy = min(x ± Dx, y ± Dy)  <- if y < x
    // TODO: this implementation is fishy, need a check, or a Monte Carlo propagation
    val::BaseValue::PointerType estimate = res1.estimate->math_min(res2.estimate.get());
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
    // val::BaseValue::PointerType estimate = min(res1.estimate, res2.estimate);
    // return puq::Result(estimate, (estimate == res1.estimate) ? res1.uncertainty : res2.uncertainty);
  }

  extern puq::Measurement min(const puq::Measurement& msr1, const puq::Measurement& msr2) {
    puq::Dimensions dim1 = msr1.baseunits.dimensions();
    puq::Dimensions dim2 = msr2.baseunits.dimensions();
    if (dim1.has_dimensions() && dim2.has_dimensions()) {
      if (dim1 == dim2) {
        return puq::Measurement(min(msr1.result, msr2.result),
                                msr1.baseunits);
      } else {
        puq::Measurement msr3 = msr2.convert(msr1.baseunits);
        return puq::Measurement(min(msr1.result, msr3.result),
                                msr1.baseunits);
      }
    } else if (dim1.has_dimensions() || dim2.has_dimensions()) {
      throw std::runtime_error("Cannot convert between dimensional and dimensionless quantities.");
    } else {
      return puq::Measurement(min(msr1.result, msr2.result));
    }
  }

  puq::Quantity min(const puq::Quantity& quant1, const puq::Quantity& quant2) {
    return puq::Quantity(min(quant1.measurement, quant2.measurement),
                         quant1.stype);
  }

} // namespace snt::puq::math
