#include <snt/puq/nostd/exp.h>

namespace snt::nostd {

  puq::MagnitudeFloat exp(const puq::MagnitudeFloat& e) {
    return std::exp(e);
  }

  val::BaseValue::PointerType exp(val::BaseValue::PointerType a) {
    return a->math_exp();
  }

  puq::Magnitude exp(const puq::Magnitude& e) {
    // z ± Dz = pow(e, y ± Dy) -> Dz = pow(e, y) * log(e) * Dy
    if (e.uncertainty)
      return puq::Magnitude(e.estimate->math_exp(), e.estimate->math_exp()->math_mul(e.uncertainty.get()));
    else
      return puq::Magnitude(e.estimate->math_exp());
    //return puq::Magnitude(exp(e.estimate), exp(e.estimate) * e.uncertainty);
  }

  puq::Measurement exp(const puq::Measurement& uv) {
    return puq::Measurement(exp(uv.magnitude), uv.baseunits);
  }

} // namespace snt::nostd
