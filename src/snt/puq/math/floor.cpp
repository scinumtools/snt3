#include <snt/puq/math/floor.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

  puq::Result floor(const puq::Result& res) {
    auto est = res.estimate->math_floor();

    if (!res.uncertainty) {
      return puq::Result(std::move(est));
    }

    // Compute interval bounds
    auto lower = res.estimate->math_sub(res.uncertainty.get());
    auto upper = res.estimate->math_add(res.uncertainty.get());

    auto floor_lower = lower->math_floor();
    auto floor_upper = upper->math_floor();

    // TODO: This should be evaluated piecewise and not all at once
    if (floor_lower->compare_equal(floor_upper.get())->any_of()) {
      // No integer boundary crossed → stable
      return puq::Result(std::move(est), nullptr);
    } else {
      // Discontinuity → result can jump
      return puq::Result(std::move(est), res.uncertainty->math_inf());
    }
  }

  puq::Measurement floor(const puq::Measurement& msr) {
    return puq::Measurement(floor(msr.result),
                            msr.baseunits);
  }

  puq::Quantity floor(const puq::Quantity& quant) {
    return puq::Quantity(floor(quant.measurement),
                         quant.stype);
  }

} // namespace snt::puq::math
