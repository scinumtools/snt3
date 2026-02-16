#include <snt/puq/nostd/log10.h>

namespace snt::nostd {

  puq::MagnitudeFloat log10(const puq::MagnitudeFloat& m) {
    return std::log10(m);
  }

#if defined(MAGNITUDE_VALUES)
  val::BaseValue::PointerType log10(val::BaseValue::PointerType a) {
    return a->math_log10();
  }
#endif

#ifdef MAGNITUDE_ERRORS
  puq::Magnitude log10(const puq::Magnitude& m) {
    // y ± Dy = log(x ± Dx) -> Dy = 1 / ln(10) * Dx / x
#ifdef MAGNITUDE_VALUES
    if (m.error) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(std::log(10));
      return puq::Magnitude(m.value->math_log10(),
                            m.error->math_div(m.value->math_mul(cst.get()).get()));
    } else {
      return puq::Magnitude(m.value->math_log10());
    }
#else
    return puq::Magnitude(log10(m.value), m.error / (m.value * std::log(10)));
#endif
  }
#endif

} // namespace snt::nostd
