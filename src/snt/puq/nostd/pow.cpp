#include <snt/puq/nostd/pow.h>

namespace snt::nostd {

  puq::MagnitudeFloat pow(const puq::MagnitudeFloat& m, const puq::ExponentFloat& e) {
    return std::pow(m, e);
  }

  val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e) {
    return a->math_pow(e.get());
  }

  puq::Magnitude pow(const puq::Magnitude& m, const puq::ExponentFloat& e) {
    // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
    if (m.uncertainty) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(e);
      return puq::Magnitude(m.estimate->math_pow(e), m.estimate->math_pow(e - 1)->math_mul(cst.get())->math_abs()->math_mul(m.uncertainty.get()));
    } else {
      return puq::Magnitude(m.estimate->math_pow(e));
    }
    //return puq::Magnitude(pow(m.estimate, e), abs(e * pow(m.estimate, e - 1)) * m.uncertainty);
  }
  
  puq::Magnitude pow(const puq::Magnitude& m, const puq::Magnitude& e) {
    // Dz = sqrt(pow(Dzx,2)+pow(Dzy,2))
    // z ± Dz = pow(x ± Dx, y ± Dy)
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(-1);
    if (m.uncertainty && e.uncertainty) {
      val::BaseValue::PointerType Dzx = e.estimate->math_mul(m.estimate->math_pow(e.estimate->math_add(cst.get()).get()).get())->math_mul(m.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      val::BaseValue::PointerType Dzy = m.estimate->math_pow(e.estimate.get())->math_mul(m.estimate->math_log().get())->math_mul(e.uncertainty.get());          // Dzy = pow(x, y) * log(x) * Dy
      return puq::Magnitude(m.estimate->math_pow(e.estimate.get()), Dzx->math_pow(2)->math_add(Dzy->math_pow(2).get())->math_sqrt());
    } else if (m.uncertainty) {
      val::BaseValue::PointerType Dzx = e.estimate->math_mul(m.estimate->math_pow(e.estimate->math_add(cst.get()).get()).get())->math_mul(m.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      return puq::Magnitude(m.estimate->math_pow(e.estimate.get()), Dzx->math_pow(2)->math_sqrt());
    } else if (e.uncertainty) {
      val::BaseValue::PointerType Dzy = m.estimate->math_pow(e.estimate.get())->math_mul(m.estimate->math_log().get())->math_mul(e.uncertainty.get()); // Dzy = pow(x, y) * log(x) * Dy
      return puq::Magnitude(m.estimate->math_pow(e.estimate.get()), Dzy->math_pow(2)->math_sqrt());
    } else {
      return puq::Magnitude(m.estimate->math_pow(e.estimate.get()));
    }
    //val::BaseValue::PointerType Dzx = e.estimate * pow(m.estimate, e.estimate - 1) * m.uncertainty;  // Dzx = y * pow(x, y-1) * Dx
    //val::BaseValue::PointerType Dzy = pow(m.estimate, e.estimate) * log(m.estimate) * e.uncertainty; // Dzy = pow(x, y) * log(x) * Dy
    //return puq::Magnitude(pow(m.estimate, e.estimate), sqrt(Dzx * Dzx + Dzy * Dzy));
  }

} // namespace snt::nostd
