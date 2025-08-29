#include "nostd.h"

namespace nostd {

    MAGNITUDE_PRECISION log(const MAGNITUDE_PRECISION& m) {
      return std::log(m);
    }

#if defined(MAGNITUDE_ARRAYS)
    Array log(const Array& a) {
      ArrayValue av(a.size());
      for (int i = 0; i < a.size(); i++)
        av[i] = std::log(a[i]);
      return Array(av, a.shape());
    }
#elif defined(MAGNITUDE_VALUES)
    val::BaseValue::PointerType log(val::BaseValue::PointerType a) {
      return a->math_log();
    }
#endif

#ifdef MAGNITUDE_ERRORS
    Magnitude log(const Magnitude& m) {
      // y ± Dy = log10(x ± Dx) -> Dy = Dx / x
#ifdef MAGNITUDE_VALUES
      if (m.error)
        return Magnitude(m.value->math_log(), m.error->math_div(m.value.get()));
      else
        return Magnitude(m.value->math_log());
#else
      return Magnitude(log(m.value), m.error / m.value);
#endif
    }
#endif

} // namespace nostd
