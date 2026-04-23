#include <snt/puq/exponent.h>
#include <snt/puq/math/tan.h>
#include <snt/puq/result.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>

namespace snt::puq::math {

  puq::Result tan(const puq::Result& m) {    
    // y ± Dy = tan(x ± Dx) -> Dy = |sec^2(x)| * Dx = (1 / cos^2(x)) * Dx
    if (m.uncertainty) {
      // Treating limits where cos(x)->0, or 1/cos2(x)->inf
      val::BaseValue::PointerType cosx = m.estimate->math_cos();
      val::ArrayValue<double> eps(1e-12); // defining a small threshold
      if (cosx->math_abs()->compare_less(&eps)->any_of()) {
	// TODO: set inf values only to those array values which are truly less than eps
	return puq::Result(m.estimate->math_tan(),
			   m.uncertainty->math_inf());
      } else {
	val::BaseValue::PointerType invcos2 = m.estimate->math_cos()->math_pow(2)->math_inv();
	return puq::Result(m.estimate->math_tan(),
			   m.uncertainty->math_mul(invcos2.get()));
      }
    } else {
      return puq::Result(m.estimate->math_tan());
    }
  }

  puq::Measurement tan(const puq::Measurement& msr) {
    if (msr.baseunits.has_dimensions())   // TODO: allow for radians
      throw std::runtime_error("Tangens function accepts only dimensionless quantities.");
    return puq::Measurement(tan(msr.result),
			    msr.baseunits);
  }

  puq::Quantity tan(const puq::Quantity& quant) {
    return puq::Quantity(tan(quant.measurement),
			 quant.stype);
  }
  
} // namespace snt::puq::math
