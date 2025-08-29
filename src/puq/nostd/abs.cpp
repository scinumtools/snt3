#include "nostd.h"

namespace nostd {

    MAGNITUDE_PRECISION abs(const MAGNITUDE_PRECISION& m) {
      return std::abs(m);
    }

#if defined(MAGNITUDE_ARRAYS)
    Array abs(const Array& a) {
      ArrayValue av(a.size());
      for (int i = 0; i < a.size(); i++)
        av[i] = std::abs(a[i]);
      return Array(av, a.shape());
    }
#elif defined(MAGNITUDE_VALUES)
    val::BaseValue::PointerType abs(val::BaseValue::PointerType a) {
      return a->math_abs();
    }
#endif

#ifdef MAGNITUDE_ERRORS
    Magnitude abs(const Magnitude& m) {
      // abs(y ± Dy) = abs(y) ± Dy
#ifdef MAGNITUDE_VALUES
      return Magnitude(m.value->math_abs(), m.error->clone());
#else
      return Magnitude(abs(m.value), m.error);
#endif
    }
#endif

} // namespace nostd
