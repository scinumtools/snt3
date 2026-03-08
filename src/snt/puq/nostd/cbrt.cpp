#include <snt/puq/nostd/cbrt.h>

namespace snt::nostd {

  puq::MagnitudeFloat cbrt(const puq::MagnitudeFloat& m) {
    return std::cbrt(m);
  }

  val::BaseValue::PointerType cbrt(val::BaseValue::PointerType a) {
    return a->math_cbrt();
  }

  puq::Magnitude cbrt(const puq::Magnitude& m) {
    // y ± Dz = pow(x ± Dx, 1/3) -> Dy = 1/3 * pow(x, -2/3) * Dx
    std::unique_ptr<val::ArrayValue<double>> third1 = std::make_unique<val::ArrayValue<double>>(1. / 3.);
    std::unique_ptr<val::ArrayValue<double>> third2 = std::make_unique<val::ArrayValue<double>>(-2 * 1. / 3.);
    return puq::Magnitude(m.estimate->math_cbrt(), m.estimate->math_pow(third2.get())->math_mul(third1.get())->math_mul(m.uncertainty.get()));
    //constexpr puq::MagnitudeFloat third = 1. / 3.;
    //return puq::Magnitude(cbrt(m.estimate), third * pow(m.estimate, -2 * third) * m.uncertainty);
  }

} // namespace snt::nostd
