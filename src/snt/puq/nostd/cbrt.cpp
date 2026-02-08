#include "cbrt.h"

namespace snt::nostd {

  puq::MagnitudeFloat cbrt(const puq::MagnitudeFloat& m) {
    return std::cbrt(m);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType cbrt(val::BaseValue::PointerType a) {
    return a->math_cbrt();
  }
#endif

#ifdef MAGNITUDE_ERRORS
  puq::Magnitude cbrt(const puq::Magnitude& m) {
    // y ± Dz = pow(x ± Dx, 1/3) -> Dy = 1/3 * pow(x, -2/3) * Dx
#ifdef MAGNITUDE_VALUES
    std::unique_ptr<val::ArrayValue<double>> third1 = std::make_unique<val::ArrayValue<double>>(1. / 3.);
    std::unique_ptr<val::ArrayValue<double>> third2 = std::make_unique<val::ArrayValue<double>>(-2 * 1. / 3.);
    return puq::Magnitude(m.value->math_cbrt(), m.value->math_pow(third2.get())->math_mul(third1.get())->math_mul(m.error.get()));
#else
    constexpr puq::MagnitudeFloat third = 1. / 3.;
    return puq::Magnitude(cbrt(m.value), third * pow(m.value, -2 * third) * m.error);
#endif
  }
#endif

} // namespace snt::nostd
