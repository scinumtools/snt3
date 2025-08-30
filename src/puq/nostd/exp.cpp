#include "nostd.h"

namespace nostd {

  MAGNITUDE_PRECISION exp(const MAGNITUDE_PRECISION& e) {
    return std::exp(e);
  }

#if defined(MAGNITUDE_ARRAYS)
  Array exp(const Array& e) {
    ArrayValue av(e.size());
    for (int i = 0; i < e.size(); i++)
      av[i] = std::exp(e[i]);
    return Array(av, e.shape());
  }
#elif defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType exp(val::BaseValue::PointerType a) {
    return a->math_exp();
  }
#endif

#ifdef MAGNITUDE_ERRORS
  puq::Magnitude exp(const puq::Magnitude& e) {
    // z ± Dz = pow(e, y ± Dy) -> Dz = pow(e, y) * log(e) * Dy
#ifdef MAGNITUDE_VALUES
    if (e.error)
      return puq::Magnitude(e.value->math_exp(), e.value->math_exp()->math_mul(e.error.get()));
    else
      return puq::Magnitude(e.value->math_exp());
#else
    return puq::Magnitude(exp(e.value), exp(e.value) * e.error);
#endif
  }
#endif

  puq::UnitValue exp(const puq::UnitValue& uv) {
    return puq::UnitValue(exp(uv.magnitude), uv.baseunits);
  }

} // namespace nostd
