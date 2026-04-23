#include <snt/puq/math/ceil.h>
#include <snt/puq/result.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>

namespace snt::puq::math {

  puq::Result ceil(const puq::Result& res) {
    auto est = res.estimate->math_ceil();
    
    if (!res.uncertainty) {
      return puq::Result(std::move(est));
    }
    
    // Compute interval bounds
    auto lower = res.estimate->math_sub(res.uncertainty.get());
    auto upper = res.estimate->math_add(res.uncertainty.get());
    
    auto ceil_lower = lower->math_ceil();
    auto ceil_upper = upper->math_ceil();

    // TODO: This should be evaluated piecewise and not all at once
    if (ceil_lower->compare_equal(ceil_upper.get())->any_of()) {
      // No integer boundary crossed → stable
      return puq::Result(std::move(est), nullptr);
    } else {
      // Discontinuity → result can jump
      return puq::Result(std::move(est), res.uncertainty->math_inf());
    }
  }

  puq::Measurement ceil(const puq::Measurement& msr) {
    return puq::Measurement(ceil(msr.result),
			    msr.baseunits);
  }

  puq::Quantity ceil(const puq::Quantity& quant) {
    return puq::Quantity(ceil(quant.measurement),
			 quant.stype);
  }
    
} // namespace snt::puq::math
