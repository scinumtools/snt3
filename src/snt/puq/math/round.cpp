#include <snt/puq/math/round.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

  puq::Result round(const puq::Result& res) {
    val::BaseValue::PointerType est = res.estimate->math_round();

    if (!res.uncertainty) {
      return puq::Result(std::move(est), nullptr);
    }

    // Shift by 0.5 to reuse floor-style boundary detection
    val::ArrayValue<double> half(0.5);

    // Compute interval bounds
    auto lower = res.estimate->math_sub(res.uncertainty.get())->math_add(&half);
    auto upper = res.estimate->math_add(res.uncertainty.get())->math_add(&half);

    auto floor_lower = lower->math_floor();
    auto floor_upper = upper->math_floor();

    // TODO: This should be evaluated piecewise and not all at once
    if (floor_lower->compare_equal(floor_upper.get())->any_of()) {
      // No integer boundary crossed → stable
      return puq::Result(std::move(est), nullptr);
    } else {
      // Discontinuity → result can jump
      return puq::Result(std::move(est), std::make_unique<val::ArrayValue<double>>(half));
    }
  }

  puq::Measurement round(const puq::Measurement& msr) {
    return puq::Measurement(round(msr.result),
                            msr.baseunits);
  }

  puq::Quantity round(const puq::Quantity& quant) {
    return puq::Quantity(round(quant.measurement),
                         quant.stype);
  }

} // namespace snt::puq::math
