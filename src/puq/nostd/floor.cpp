#include "nostd.h"

namespace nostd {

    MAGNITUDE_PRECISION floor(const MAGNITUDE_PRECISION& m) {
      return std::floor(m);
    }

#if defined(MAGNITUDE_ARRAYS)
    Array floor(const Array& a) {
      ArrayValue av(a.size());
      for (int i = 0; i < a.size(); i++)
        av[i] = std::floor(a[i]);
      return Array(av, a.shape());
    }
#elif defined(MAGNITUDE_VALUES)
    val::BaseValue::PointerType floor(val::BaseValue::PointerType a) {
      return a->math_floor();
    }
#endif

#ifdef MAGNITUDE_ERRORS
    Magnitude floor(const Magnitude& m) {
#ifdef MAGNITUDE_VALUES
      return Magnitude(m.value->math_floor());
#else
      return Magnitude(floor(m.value));
#endif
    }
#endif

} // namespace nostd
