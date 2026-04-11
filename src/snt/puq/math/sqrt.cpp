#include <snt/puq/math/sqrt.h>

namespace snt::math {

  double sqrt(const double m) {
    return std::sqrt(m);
  }

  val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a) {
    return a->math_sqrt();
  }

  puq::Result sqrt(const puq::Result& m) {
    // y ± Dz = pow(x ± Dx, 0.5) -> Dy = 0.5 * pow(x, -0.5) * Dx
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(0.5);
    return puq::Result(m.estimate->math_sqrt(), m.estimate->math_pow(-0.5)->math_mul(cst.get())->math_mul(m.uncertainty.get()));
    // return puq::Result(sqrt(m.estimate), 0.5 * pow(m.estimate, -0.5) * m.uncertainty);
  }

  puq::Measurement sqrt(const puq::Measurement& msr) {
    // y ± Dz = pow(x ± Dx, 0.5) -> Dy = 0.5 * pow(x, -0.5) * Dx
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(0.5);
    return puq::Measurement(msr.magnitude.estimate->math_sqrt(), msr.magnitude.estimate->math_pow(-0.5)->math_mul(cst.get())->math_mul(msr.magnitude.uncertainty.get()));
    // return puq::Measurement(sqrt(msr.magnitude.estimate), 0.5 * pow(msr.magnitude.estimate, -0.5) * msr.magnitude.uncertainty);
  }

} // namespace snt::math
