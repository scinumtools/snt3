#include <snt/puq/math/pow.h>

namespace snt::puq::math {

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
    if (msr.result.uncertainty) {
      std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(e);
      return puq::Measurement(msr.result.estimate->math_pow(e), msr.result.estimate->math_pow(e - 1)->math_mul(cst.get())->math_abs()->math_mul(msr.result.uncertainty.get()));
    } else {
      return puq::Measurement(msr.result.estimate->math_pow(e));
    }
    // return puq::Measurement(pow(msr.result.estimate, e), abs(e * pow(msr.result.estimate, e - 1)) * msr.result.uncertainty);
  }

  puq::Measurement pow(const puq::Measurement& msr1, const puq::Measurement& msr2) {
    // Dz = sqrt(pow(Dzx,2)+pow(Dzy,2))
    // z ± Dz = pow(x ± Dx, y ± Dy)
    std::unique_ptr<val::ArrayValue<double>> cst = std::make_unique<val::ArrayValue<double>>(-1);
    if (msr1.result.uncertainty && msr2.result.uncertainty) {
      val::BaseValue::PointerType Dzx = msr2.result.estimate->math_mul(msr1.result.estimate->math_pow(msr2.result.estimate->math_add(cst.get()).get()).get())->math_mul(msr1.result.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      val::BaseValue::PointerType Dzy = msr1.result.estimate->math_pow(msr2.result.estimate.get())->math_mul(msr1.result.estimate->math_log().get())->math_mul(msr2.result.uncertainty.get());          // Dzy = pow(x, y) * log(x) * Dy
      return puq::Measurement(msr1.result.estimate->math_pow(msr2.result.estimate.get()), Dzx->math_pow(2)->math_add(Dzy->math_pow(2).get())->math_sqrt());
    } else if (msr1.result.uncertainty) {
      val::BaseValue::PointerType Dzx = msr2.result.estimate->math_mul(msr1.result.estimate->math_pow(msr2.result.estimate->math_add(cst.get()).get()).get())->math_mul(msr1.result.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
      return puq::Measurement(msr1.result.estimate->math_pow(msr2.result.estimate.get()), Dzx->math_pow(2)->math_sqrt());
    } else if (msr2.result.uncertainty) {
      val::BaseValue::PointerType Dzy = msr1.result.estimate->math_pow(msr2.result.estimate.get())->math_mul(msr1.result.estimate->math_log().get())->math_mul(msr2.result.uncertainty.get()); // Dzy = pow(x, y) * log(x) * Dy
      return puq::Measurement(msr1.result.estimate->math_pow(msr2.result.estimate.get()), Dzy->math_pow(2)->math_sqrt());
    } else {
      return puq::Measurement(msr1.result.estimate->math_pow(msr2.result.estimate.get()));
    }
    // val::BaseValue::PointerType Dzx = msr2.result.estimate * pow(msr1.result.estimate, msr2.result.estimate - 1) * msr1.result.uncertainty;  // Dzx = y * pow(x, y-1) * Dx
    // val::BaseValue::PointerType Dzy = pow(msr1.result.estimate, msr2.result.estimate) * log(msr1.result.estimate) * msr2.result.uncertainty; // Dzy = pow(x, y) * log(x) * Dy
    // return puq::Measurement(pow(msr1.result.estimate, msr2.result.estimate), sqrt(Dzx * Dzx + Dzy * Dzy));
  }

} // namespace snt::puq::math
