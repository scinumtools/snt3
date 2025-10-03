#include "pow.h"

namespace snt::nostd {

  MAGNITUDE_PRECISION pow(const MAGNITUDE_PRECISION& m, const EXPONENT_FLOAT_PRECISION& e) {
    return std::pow(m, e);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e) {
    return a->math_pow(e.get());
  }
#endif

#ifdef MAGNITUDE_ERRORS
  puq::Magnitude pow(const puq::Magnitude& m, const EXPONENT_FLOAT_PRECISION& e) {
#ifdef MAGNITUDE_VALUES
    // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
    if (m.error) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(e);
      return puq::Magnitude(m.value->math_pow(e), m.value->math_pow(e - 1)->math_mul(cst.get())->math_abs()->math_mul(m.error.get()));
    } else {
      return puq::Magnitude(m.value->math_pow(e));
    }
#else
    // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
    return puq::Magnitude(pow(m.value, e), abs(e * pow(m.value, e - 1)) * m.error);
#endif
  }
  puq::Magnitude pow(const puq::Magnitude& m, const puq::Magnitude& e) {
    // Dz = sqrt(pow(Dzx,2)+pow(Dzy,2))
    // z ± Dz = pow(x ± Dx, y ± Dy)
#ifdef MAGNITUDE_VALUES
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(-1);
    if (m.error && e.error) {
      MAGNITUDE_VALUE Dzx = e.value->math_mul(m.value->math_pow(e.value->math_add(cst.get()).get()).get())->math_mul(m.error.get()); // Dzx = y * pow(x, y-1) * Dx
      MAGNITUDE_VALUE Dzy = m.value->math_pow(e.value.get())->math_mul(m.value->math_log().get())->math_mul(e.error.get());          // Dzy = pow(x, y) * log(x) * Dy
      return puq::Magnitude(m.value->math_pow(e.value.get()), Dzx->math_pow(2)->math_add(Dzy->math_pow(2).get())->math_sqrt());
    } else if (m.error) {
      MAGNITUDE_VALUE Dzx = e.value->math_mul(m.value->math_pow(e.value->math_add(cst.get()).get()).get())->math_mul(m.error.get()); // Dzx = y * pow(x, y-1) * Dx
      return puq::Magnitude(m.value->math_pow(e.value.get()), Dzx->math_pow(2)->math_sqrt());
    } else if (e.error) {
      MAGNITUDE_VALUE Dzy = m.value->math_pow(e.value.get())->math_mul(m.value->math_log().get())->math_mul(e.error.get()); // Dzy = pow(x, y) * log(x) * Dy
      return puq::Magnitude(m.value->math_pow(e.value.get()), Dzy->math_pow(2)->math_sqrt());
    } else {
      return puq::Magnitude(m.value->math_pow(e.value.get()));
    }
#else
    MAGNITUDE_VALUE Dzx = e.value * pow(m.value, e.value - 1) * m.error;  // Dzx = y * pow(x, y-1) * Dx
    MAGNITUDE_VALUE Dzy = pow(m.value, e.value) * log(m.value) * e.error; // Dzy = pow(x, y) * log(x) * Dy
    return puq::Magnitude(pow(m.value, e.value), sqrt(Dzx * Dzx + Dzy * Dzy));
#endif
  }
#endif

} // namespace snt::nostd
