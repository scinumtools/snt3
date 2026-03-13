#include <snt/puq/nostd/cbrt.h>

namespace snt::nostd {

  double cbrt(const double& m) {
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
    //constexpr double third = 1. / 3.;
    //return puq::Magnitude(cbrt(m.estimate), third * pow(m.estimate, -2 * third) * m.uncertainty);
  }

  puq::Measurement cbrt(const puq::Measurement& msr) {
    // y ± Dz = pow(x ± Dx, 1/3) -> Dy = 1/3 * pow(x, -2/3) * Dx
    std::unique_ptr<val::ArrayValue<double>> third1 = std::make_unique<val::ArrayValue<double>>(1. / 3.);
    std::unique_ptr<val::ArrayValue<double>> third2 = std::make_unique<val::ArrayValue<double>>(-2 * 1. / 3.);
    return puq::Measurement(msr.magnitude.estimate->math_cbrt(), msr.magnitude.estimate->math_pow(third2.get())->math_mul(third1.get())->math_mul(msr.magnitude.uncertainty.get()));
    //constexpr puq::MeasurementFloat third = 1. / 3.;
    //return puq::Measurement(cbrt(msr.magnitude.estimate), third * pow(msr.magnitude.estimate, -2 * third) * msr.magnitude.uncertainty);
  }

} // namespace snt::nostd
