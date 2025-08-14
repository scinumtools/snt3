#include "nostd.h"

namespace puq {
  namespace nostd {

    MAGNITUDE_PRECISION pow(const MAGNITUDE_PRECISION& m, const EXPONENT_REAL_PRECISION& e) {
      return std::pow(m, e);
    }

#if defined(MAGNITUDE_ARRAYS)
    Array pow(const Array& a, const EXPONENT_REAL_PRECISION& e) {
      ArrayValue av(a.size());
      for (int i = 0; i < a.size(); i++) {
        av[i] = std::pow(a[i], e);
      }
      return Array(av, a.shape());
    }
    Array pow(const Array& a, const Array& e) {
      auto fn = [](const MAGNITUDE_PRECISION& v1, const MAGNITUDE_PRECISION& v2) {
        return std::pow(v1, v2);
      };
      return Array::const_operation(a, e, fn);
    }
#elif defined(MAGNITUDE_VALUES)
    val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e) {
      return a->math_pow(e.get());
    }
#endif

#ifdef MAGNITUDE_ERRORS
    Magnitude pow(const Magnitude& m, const EXPONENT_REAL_PRECISION& e) {
#ifdef MAGNITUDE_VALUES
      // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
      if (m.error)
        return Magnitude(m.value->math_pow(e), m.value->math_pow(e - 1)->math_mul(e)->math_abs()->math_mul(m.error.get()));
      else
        return Magnitude(m.value->math_pow(e));
#else
      // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
      return Magnitude(pow(m.value, e), abs(e * pow(m.value, e - 1)) * m.error);
#endif
    }
    Magnitude pow(const Magnitude& m, const Magnitude& e) {
      // Dz = sqrt(pow(Dzx,2)+pow(Dzy,2))
      // z ± Dz = pow(x ± Dx, y ± Dy)
#ifdef MAGNITUDE_VALUES
      if (m.error && e.error) {
        MAGNITUDE_VALUE Dzx = e.value->math_mul(m.value->math_pow(e.value->math_add(-1).get()).get())->math_mul(m.error.get()); // Dzx = y * pow(x, y-1) * Dx
        MAGNITUDE_VALUE Dzy = m.value->math_pow(e.value.get())->math_mul(m.value->math_log().get())->math_mul(e.error.get());   // Dzy = pow(x, y) * log(x) * Dy
        return Magnitude(m.value->math_pow(e.value.get()), Dzx->math_pow(2)->math_add(Dzy->math_pow(2).get())->math_sqrt());
      } else if (m.error) {
        MAGNITUDE_VALUE Dzx = e.value->math_mul(m.value->math_pow(e.value->math_add(-1).get()).get())->math_mul(m.error.get()); // Dzx = y * pow(x, y-1) * Dx
        return Magnitude(m.value->math_pow(e.value.get()), Dzx->math_pow(2)->math_sqrt());
      } else if (e.error) {
        MAGNITUDE_VALUE Dzy = m.value->math_pow(e.value.get())->math_mul(m.value->math_log().get())->math_mul(e.error.get()); // Dzy = pow(x, y) * log(x) * Dy
        return Magnitude(m.value->math_pow(e.value.get()), Dzy->math_pow(2)->math_sqrt());
      } else {
        return Magnitude(m.value->math_pow(e.value.get()));
      }
#else
      MAGNITUDE_VALUE Dzx = e.value * pow(m.value, e.value - 1) * m.error;  // Dzx = y * pow(x, y-1) * Dx
      MAGNITUDE_VALUE Dzy = pow(m.value, e.value) * log(m.value) * e.error; // Dzy = pow(x, y) * log(x) * Dy
      return Magnitude(pow(m.value, e.value), sqrt(Dzx * Dzx + Dzy * Dzy));
#endif
    }
#endif

  } // namespace nostd
} // namespace puq
