#include "sqrt.h"

namespace snt::nostd {

  MagnitudeFloat sqrt(const MagnitudeFloat& m) {
    return std::sqrt(m);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a) {
    return a->math_sqrt();
  }
#endif

#ifdef MAGNITUDE_ERRORS
  puq::Magnitude sqrt(const puq::Magnitude& m) {
    // y ± Dz = pow(x ± Dx, 0.5) -> Dy = 0.5 * pow(x, -0.5) * Dx
#ifdef MAGNITUDE_VALUES
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(0.5);
    return puq::Magnitude(m.value->math_sqrt(), m.value->math_pow(-0.5)->math_mul(cst.get())->math_mul(m.error.get()));
#else
    return puq::Magnitude(sqrt(m.value), 0.5 * pow(m.value, -0.5) * m.error);
#endif
  }
#endif

} // namespace snt::nostd
