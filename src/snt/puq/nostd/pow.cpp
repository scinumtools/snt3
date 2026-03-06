#include <snt/puq/nostd/pow.h>

namespace snt::nostd {

  puq::MagnitudeFloat pow(const puq::MagnitudeFloat& m, const puq::ExponentFloat& e) {
    return std::pow(m, e);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e) {
    return a->math_pow(e.get());
  }
#endif

  puq::Magnitude pow(const puq::Magnitude& m, const puq::ExponentFloat& e) {
#ifdef MAGNITUDE_VALUES
    // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
    if (m.uncertainty) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(e);
      return puq::Magnitude(m.estimate->math_pow(e), m.estimate->math_pow(e - 1)->math_mul(cst.get())->math_abs()->math_mul(m.uncertainty.get()));
    } else {
      return puq::Magnitude(m.estimate->math_pow(e));
    }
#else
    // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
    return puq::Magnitude(pow(m.estimate, e), abs(e * pow(m.estimate, e - 1)) * m.uncertainty);
#endif
  }
  
  puq::Magnitude pow(const puq::Magnitude& m, const puq::Magnitude& e) {
    // Dz = sqrt(pow(Dzx,2)+pow(Dzy,2))
    // z ± Dz = pow(x ± Dx, y ± Dy)
#ifdef MAGNITUDE_VALUES
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(-1);
    if (m.uncertainty && e.uncertainty) {
      MAGNITUDE_VALUE Dzx = e.estimate->math_mul(m.estimate->math_pow(e.estimate->math_add(cst.get()).get()).get())->math_mul(m.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      MAGNITUDE_VALUE Dzy = m.estimate->math_pow(e.estimate.get())->math_mul(m.estimate->math_log().get())->math_mul(e.uncertainty.get());          // Dzy = pow(x, y) * log(x) * Dy
      return puq::Magnitude(m.estimate->math_pow(e.estimate.get()), Dzx->math_pow(2)->math_add(Dzy->math_pow(2).get())->math_sqrt());
    } else if (m.uncertainty) {
      MAGNITUDE_VALUE Dzx = e.estimate->math_mul(m.estimate->math_pow(e.estimate->math_add(cst.get()).get()).get())->math_mul(m.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      return puq::Magnitude(m.estimate->math_pow(e.estimate.get()), Dzx->math_pow(2)->math_sqrt());
    } else if (e.uncertainty) {
      MAGNITUDE_VALUE Dzy = m.estimate->math_pow(e.estimate.get())->math_mul(m.estimate->math_log().get())->math_mul(e.uncertainty.get()); // Dzy = pow(x, y) * log(x) * Dy
      return puq::Magnitude(m.estimate->math_pow(e.estimate.get()), Dzy->math_pow(2)->math_sqrt());
    } else {
      return puq::Magnitude(m.estimate->math_pow(e.estimate.get()));
    }
#else
    MAGNITUDE_VALUE Dzx = e.estimate * pow(m.estimate, e.estimate - 1) * m.uncertainty;  // Dzx = y * pow(x, y-1) * Dx
    MAGNITUDE_VALUE Dzy = pow(m.estimate, e.estimate) * log(m.estimate) * e.uncertainty; // Dzy = pow(x, y) * log(x) * Dy
    return puq::Magnitude(pow(m.estimate, e.estimate), sqrt(Dzx * Dzx + Dzy * Dzy));
#endif
  }

} // namespace snt::nostd
