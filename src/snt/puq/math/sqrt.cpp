#include <snt/puq/math/sqrt.h>

namespace snt::puq::math {

  puq::Result sqrt(const puq::Result& m) {
    // y ± Dz = pow(x ± Dx, 0.5) -> Dy = 0.5 * pow(x, -0.5) * Dx
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(0.5);
    return puq::Result(m.estimate->math_sqrt(), m.estimate->math_pow(-0.5)->math_mul(cst.get())->math_mul(m.uncertainty.get()));
    // return puq::Result(sqrt(m.estimate), 0.5 * pow(m.estimate, -0.5) * m.uncertainty);
  }

  puq::Measurement sqrt(const puq::Measurement& msr) {
    // y ± Dz = pow(x ± Dx, 0.5) -> Dy = 0.5 * pow(x, -0.5) * Dx
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(0.5);
    return puq::Measurement(msr.result.estimate->math_sqrt(), msr.result.estimate->math_pow(-0.5)->math_mul(cst.get())->math_mul(msr.result.uncertainty.get()));
    // return puq::Measurement(sqrt(msr.result.estimate), 0.5 * pow(msr.result.estimate, -0.5) * msr.result.uncertainty);
  }

} // namespace snt::puq::math
