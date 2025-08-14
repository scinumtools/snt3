#include "nostd.h"

namespace puq {
  namespace nostd {

    MAGNITUDE_PRECISION log10(const MAGNITUDE_PRECISION& m) {
      return std::log10(m);
    }

#if defined(MAGNITUDE_ARRAYS)
    Array log10(const Array& a) {
      ArrayValue av(a.size());
      for (int i = 0; i < a.size(); i++)
        av[i] = std::log10(a[i]);
      return Array(av, a.shape());
    }
#elif defined(MAGNITUDE_VALUES)
    val::BaseValue::PointerType log10(val::BaseValue::PointerType a) {
      return a->math_log10();
    }
#endif

#ifdef MAGNITUDE_ERRORS
    Magnitude log10(const Magnitude& m) {
      // y ± Dy = log(x ± Dx) -> Dy = 1 / ln(10) * Dx / x
#ifdef MAGNITUDE_VALUES
      if (m.error)
        return Magnitude(m.value->math_log10(),
                         m.error->math_div(m.value->math_mul(std::log(10)).get()));
      else
        return Magnitude(m.value->math_log10());
#else
      return Magnitude(log10(m.value), m.error / (m.value * std::log(10)));
#endif
    }
#endif

  } // namespace nostd
} // namespace puq
