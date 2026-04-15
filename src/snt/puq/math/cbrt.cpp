#include <snt/puq/math/cbrt.h>

namespace snt::puq::math {

  puq::Result cbrt(const puq::Result& m) {
    // y ± Dz = pow(x ± Dx, 1/3) -> Dy = 1/3 * pow(x, -2/3) * Dx
    std::unique_ptr<val::ArrayValue<double>> third1 = std::make_unique<val::ArrayValue<double>>(1. / 3.);
    std::unique_ptr<val::ArrayValue<double>> third2 = std::make_unique<val::ArrayValue<double>>(-2 * 1. / 3.);
    return puq::Result(m.estimate->math_cbrt(), m.estimate->math_pow(third2.get())->math_mul(third1.get())->math_mul(m.uncertainty.get()));
    // constexpr double third = 1. / 3.;
    // return puq::Result(cbrt(m.estimate), third * pow(m.estimate, -2 * third) * m.uncertainty);
  }

  puq::Measurement cbrt(const puq::Measurement& msr) {
    // y ± Dz = pow(x ± Dx, 1/3) -> Dy = 1/3 * pow(x, -2/3) * Dx
    std::unique_ptr<val::ArrayValue<double>> third1 = std::make_unique<val::ArrayValue<double>>(1. / 3.);
    std::unique_ptr<val::ArrayValue<double>> third2 = std::make_unique<val::ArrayValue<double>>(-2 * 1. / 3.);
    return puq::Measurement(msr.result.estimate->math_cbrt(), msr.result.estimate->math_pow(third2.get())->math_mul(third1.get())->math_mul(msr.result.uncertainty.get()));
    // constexpr puq::MeasurementFloat third = 1. / 3.;
    // return puq::Measurement(cbrt(msr.result.estimate), third * pow(msr.result.estimate, -2 * third) * msr.result.uncertainty);
  }

} // namespace snt::puq::math
