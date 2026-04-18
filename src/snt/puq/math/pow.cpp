#include <snt/puq/math/pow.h>
#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>
#include <snt/puq/quantity.h>

namespace snt::puq::math {

  puq::Result pow(const puq::Result& res, const double exp) {
    // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
    if (res.uncertainty) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(exp);
      return puq::Result(res.estimate->math_pow(exp), res.estimate->math_pow(exp - 1)->math_mul(cst.get())->math_abs()->math_mul(res.uncertainty.get()));
    } else {
      return puq::Result(res.estimate->math_pow(exp));
    }
    // return puq::Result(pow(res.estimate, exp), abs(exp * pow(res.estimate, exp - 1)) * res.uncertainty);
  }

  puq::Result pow(const puq::Result& res1, const puq::Result& res2) {
    // Dz = sqrt(pow(Dzx,2)+pow(Dzy,2))
    // z ± Dz = pow(x ± Dx, y ± Dy)
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(-1);
    if (res1.uncertainty && res2.uncertainty) {
      val::BaseValue::PointerType Dzx = res2.estimate->math_mul(res1.estimate->math_pow(res2.estimate->math_add(cst.get()).get()).get())->math_mul(res1.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      val::BaseValue::PointerType Dzy = res1.estimate->math_pow(res2.estimate.get())->math_mul(res1.estimate->math_log().get())->math_mul(res2.uncertainty.get());          // Dzy = pow(x, y) * log(x) * Dy
      return puq::Result(res1.estimate->math_pow(res2.estimate.get()), Dzx->math_pow(2)->math_add(Dzy->math_pow(2).get())->math_sqrt());
    } else if (res1.uncertainty) {
      val::BaseValue::PointerType Dzx = res2.estimate->math_mul(res1.estimate->math_pow(res2.estimate->math_add(cst.get()).get()).get())->math_mul(res1.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      return puq::Result(res1.estimate->math_pow(res2.estimate.get()), Dzx->math_pow(2)->math_sqrt());
    } else if (res2.uncertainty) {
      val::BaseValue::PointerType Dzy = res1.estimate->math_pow(res2.estimate.get())->math_mul(res1.estimate->math_log().get())->math_mul(res2.uncertainty.get()); // Dzy = pow(x, y) * log(x) * Dy
      return puq::Result(res1.estimate->math_pow(res2.estimate.get()), Dzy->math_pow(2)->math_sqrt());
    } else {
      return puq::Result(res1.estimate->math_pow(res2.estimate.get()));
    }
    // val::BaseValue::PointerType Dzx = res2.estimate * pow(res1.estimate, res2.estimate - 1) * res1.uncertainty;  // Dzx = y * pow(x, y-1) * Dx
    // val::BaseValue::PointerType Dzy = pow(res1.estimate, res2.estimate) * log(res1.estimate) * res2.uncertainty; // Dzy = pow(x, y) * log(x) * Dy
    // return puq::Result(pow(res1.estimate, res2.estimate), sqrt(Dzx * Dzx + Dzy * Dzy));
  }

  puq::Measurement pow(const puq::Measurement& msr, const double exp) {
    return puq::Measurement(pow(msr.result, exp),
			    msr.baseunits);
  }

  puq::Measurement pow(const puq::Measurement& msr1, const puq::Measurement& msr2) {
    return puq::Measurement(pow(msr1.result, msr2.result),
			    msr1.baseunits);
  }

  puq::Quantity pow(const puq::Quantity& quant, const double exp) {
    return puq::Quantity(pow(quant.measurement, exp),
			 quant.stype);
  }
  
  puq::Quantity pow(const puq::Quantity& quant1, const puq::Quantity& quant2) {
    return puq::Quantity(pow(quant1.measurement, quant2.measurement),
			 quant1.stype);
  }
  
} // namespace snt::puq::math
