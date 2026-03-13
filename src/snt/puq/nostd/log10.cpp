#include <snt/puq/nostd/log10.h>

namespace snt::nostd {

  double log10(const double m) {
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

  puq::Measurement log10(const puq::Measurement& msr) {
    // y ± Dy = log(x ± Dx) -> Dy = 1 / ln(10) * Dx / x
    if (msr.magnitude.uncertainty) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(std::log(10));
      return puq::Measurement(msr.magnitude.estimate->math_log10(),
                            msr.magnitude.uncertainty->math_div(msr.magnitude.estimate->math_mul(cst.get()).get()));
    } else {
      return puq::Measurement(msr.magnitude.estimate->math_log10());
    }
    //return puq::Measurement(log10(msr.magnitude.estimate), msr.magnitude.uncertainty / (msr.magnitude.estimate * std::log(10)));
  }

} // namespace snt::nostd
