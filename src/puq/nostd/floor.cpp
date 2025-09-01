#include "nostd.h"

namespace snt::nostd {

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
  puq::Magnitude floor(const puq::Magnitude& m) {
#ifdef MAGNITUDE_VALUES
    return puq::Magnitude(m.value->math_floor());
#else
    return puq::Magnitude(floor(m.value));
#endif
  }
#endif

} // namespace snt::nostd
