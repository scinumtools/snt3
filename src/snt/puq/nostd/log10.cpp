#include <snt/puq/nostd/log10.h>

namespace snt::nostd {

  puq::MagnitudeFloat log10(const puq::MagnitudeFloat& m) {
    return std::log10(m);
  }

  val::BaseValue::PointerType log10(val::BaseValue::PointerType a) {
    return a->math_log10();
  }

  puq::Magnitude log10(const puq::Magnitude& m) {
    // y ± Dy = log(x ± Dx) -> Dy = 1 / ln(10) * Dx / x
    if (m.uncertainty) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(std::log(10));
      return puq::Magnitude(m.estimate->math_log10(),
                            m.uncertainty->math_div(m.estimate->math_mul(cst.get()).get()));
    } else {
      return puq::Magnitude(m.estimate->math_log10());
    }
    //return puq::Magnitude(log10(m.estimate), m.uncertainty / (m.estimate * std::log(10)));
  }

} // namespace snt::nostd
