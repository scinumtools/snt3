#include <snt/puq/math/log10.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

  puq::Result log10(const puq::Result& m) {
    // y ± Dy = log(x ± Dx) -> Dy = 1 / ln(10) * Dx / x
    if (m.uncertainty) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(std::log(10));
      return puq::Result(m.estimate->math_log10(),
                         m.uncertainty->math_div(m.estimate->math_mul(cst.get()).get()));
    } else {
      return puq::Result(m.estimate->math_log10());
    }
    // return puq::Result(log10(m.estimate), m.uncertainty / (m.estimate * std::log(10)));
  }

  puq::Measurement log10(const puq::Measurement& msr) {
    if (msr.baseunits.has_dimensions())
      throw std::runtime_error("Decadic logarithm accepts only dimensionless quantities.");
    return puq::Measurement(log10(msr.result),
                            msr.baseunits);
  }

  puq::Quantity log10(const puq::Quantity& quant) {
    return puq::Quantity(log10(quant.measurement),
                         quant.stype);
  }

} // namespace snt::puq::math
