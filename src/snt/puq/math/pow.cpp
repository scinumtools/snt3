#include <snt/puq/math/pow.h>

namespace snt::math {

  double pow(const double m, const double e) {
    return std::pow(m, e);
  }

  val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e) {
    return a->math_pow(e.get());
  }

  puq::Result pow(const puq::Result& m, const double e) {
    // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
    if (m.uncertainty) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(e);
      return puq::Result(m.estimate->math_pow(e), m.estimate->math_pow(e - 1)->math_mul(cst.get())->math_abs()->math_mul(m.uncertainty.get()));
    } else {
      return puq::Result(m.estimate->math_pow(e));
    }
    // return puq::Result(pow(m.estimate, e), abs(e * pow(m.estimate, e - 1)) * m.uncertainty);
  }

  puq::Result pow(const puq::Result& m, const puq::Result& e) {
    // Dz = sqrt(pow(Dzx,2)+pow(Dzy,2))
    // z ± Dz = pow(x ± Dx, y ± Dy)
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(-1);
    if (m.uncertainty && e.uncertainty) {
      val::BaseValue::PointerType Dzx = e.estimate->math_mul(m.estimate->math_pow(e.estimate->math_add(cst.get()).get()).get())->math_mul(m.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      val::BaseValue::PointerType Dzy = m.estimate->math_pow(e.estimate.get())->math_mul(m.estimate->math_log().get())->math_mul(e.uncertainty.get());          // Dzy = pow(x, y) * log(x) * Dy
      return puq::Result(m.estimate->math_pow(e.estimate.get()), Dzx->math_pow(2)->math_add(Dzy->math_pow(2).get())->math_sqrt());
    } else if (m.uncertainty) {
      val::BaseValue::PointerType Dzx = e.estimate->math_mul(m.estimate->math_pow(e.estimate->math_add(cst.get()).get()).get())->math_mul(m.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      return puq::Result(m.estimate->math_pow(e.estimate.get()), Dzx->math_pow(2)->math_sqrt());
    } else if (e.uncertainty) {
      val::BaseValue::PointerType Dzy = m.estimate->math_pow(e.estimate.get())->math_mul(m.estimate->math_log().get())->math_mul(e.uncertainty.get()); // Dzy = pow(x, y) * log(x) * Dy
      return puq::Result(m.estimate->math_pow(e.estimate.get()), Dzy->math_pow(2)->math_sqrt());
    } else {
      return puq::Result(m.estimate->math_pow(e.estimate.get()));
    }
    // val::BaseValue::PointerType Dzx = e.estimate * pow(m.estimate, e.estimate - 1) * m.uncertainty;  // Dzx = y * pow(x, y-1) * Dx
    // val::BaseValue::PointerType Dzy = pow(m.estimate, e.estimate) * log(m.estimate) * e.uncertainty; // Dzy = pow(x, y) * log(x) * Dy
    // return puq::Result(pow(m.estimate, e.estimate), sqrt(Dzx * Dzx + Dzy * Dzy));
  }

  puq::Measurement pow(const puq::Measurement& msr, const double e) {
    // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
    if (msr.magnitude.uncertainty) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(e);
      return puq::Measurement(msr.magnitude.estimate->math_pow(e), msr.magnitude.estimate->math_pow(e - 1)->math_mul(cst.get())->math_abs()->math_mul(msr.magnitude.uncertainty.get()));
    } else {
      return puq::Measurement(msr.magnitude.estimate->math_pow(e));
    }
    // return puq::Measurement(pow(msr.magnitude.estimate, e), abs(e * pow(msr.magnitude.estimate, e - 1)) * msr.magnitude.uncertainty);
  }

  puq::Measurement pow(const puq::Measurement& msr1, const puq::Measurement& msr2) {
    // Dz = sqrt(pow(Dzx,2)+pow(Dzy,2))
    // z ± Dz = pow(x ± Dx, y ± Dy)
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(-1);
    if (msr1.magnitude.uncertainty && msr2.magnitude.uncertainty) {
      val::BaseValue::PointerType Dzx = msr2.magnitude.estimate->math_mul(msr1.magnitude.estimate->math_pow(msr2.magnitude.estimate->math_add(cst.get()).get()).get())->math_mul(msr1.magnitude.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      val::BaseValue::PointerType Dzy = msr1.magnitude.estimate->math_pow(msr2.magnitude.estimate.get())->math_mul(msr1.magnitude.estimate->math_log().get())->math_mul(msr2.magnitude.uncertainty.get());          // Dzy = pow(x, y) * log(x) * Dy
      return puq::Measurement(msr1.magnitude.estimate->math_pow(msr2.magnitude.estimate.get()), Dzx->math_pow(2)->math_add(Dzy->math_pow(2).get())->math_sqrt());
    } else if (msr1.magnitude.uncertainty) {
      val::BaseValue::PointerType Dzx = msr2.magnitude.estimate->math_mul(msr1.magnitude.estimate->math_pow(msr2.magnitude.estimate->math_add(cst.get()).get()).get())->math_mul(msr1.magnitude.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      return puq::Measurement(msr1.magnitude.estimate->math_pow(msr2.magnitude.estimate.get()), Dzx->math_pow(2)->math_sqrt());
    } else if (msr2.magnitude.uncertainty) {
      val::BaseValue::PointerType Dzy = msr1.magnitude.estimate->math_pow(msr2.magnitude.estimate.get())->math_mul(msr1.magnitude.estimate->math_log().get())->math_mul(msr2.magnitude.uncertainty.get()); // Dzy = pow(x, y) * log(x) * Dy
      return puq::Measurement(msr1.magnitude.estimate->math_pow(msr2.magnitude.estimate.get()), Dzy->math_pow(2)->math_sqrt());
    } else {
      return puq::Measurement(msr1.magnitude.estimate->math_pow(msr2.magnitude.estimate.get()));
    }
    // val::BaseValue::PointerType Dzx = msr2.magnitude.estimate * pow(msr1.magnitude.estimate, msr2.magnitude.estimate - 1) * msr1.magnitude.uncertainty;  // Dzx = y * pow(x, y-1) * Dx
    // val::BaseValue::PointerType Dzy = pow(msr1.magnitude.estimate, msr2.magnitude.estimate) * log(msr1.magnitude.estimate) * msr2.magnitude.uncertainty; // Dzy = pow(x, y) * log(x) * Dy
    // return puq::Measurement(pow(msr1.magnitude.estimate, msr2.magnitude.estimate), sqrt(Dzx * Dzx + Dzy * Dzy));
  }

} // namespace snt::math
