#include <snt/puq/math/floor.h>
#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>
#include <snt/puq/quantity.h>

namespace snt::puq::math {

  puq::Result floor(const puq::Result& res) {
    return puq::Result(res.estimate->math_floor());
    // return puq::Result(floor(res.estimate));
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
