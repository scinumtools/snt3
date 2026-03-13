#include <snt/puq/magnitude.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace snt::puq {

  Magnitude::Magnitude(val::BaseValue::PointerType m) : estimate(std::move(m)), uncertainty(nullptr) {
    if (!estimate)
      throw std::invalid_argument("Magnitude value cannot be a null pointer.");
  }

  Magnitude::Magnitude(val::BaseValue::PointerType m, val::BaseValue::PointerType e) : estimate(std::move(m)), uncertainty(std::move(e)) {
    if (!estimate)
      throw std::invalid_argument("Magnitude value cannot be a null pointer.");
    if (uncertainty && estimate->get_size() != uncertainty->get_size())
      throw std::invalid_argument("Value and uncertainty arrays have different size: " + std::to_string(estimate->get_size()) + " != " + std::to_string(uncertainty->get_size()));
  }

  /*
   * Convert absolute and relative (in %) uncertainties
   */
  double Magnitude::abs_to_rel(const double v, const double e) {
    return 100 * e / v;
  };

  double Magnitude::rel_to_abs(const double v, const double e) {
    return v * e / 100;
  };

  // DEBUG: need to implement properly
  val::BaseValue::PointerType Magnitude::abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
    return e->math_div(v.get())->math_mul(std::make_unique<val::ArrayValue<double>>(100).get());
  };

  val::BaseValue::PointerType Magnitude::rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
    return v->math_mul(e.get())->math_div(std::make_unique<val::ArrayValue<double>>(100).get());
  };

  /*
   * Return a string representation of a magnitude
   */
  std::string Magnitude::to_string(const UnitFormat& format) const {
    std::stringstream ss;
    snt::StringFormatType fmt;
    fmt.valuePrecision = format.precision;
    if (uncertainty == nullptr || !format.display_uncertainty()) {
      ss << estimate->to_string(fmt);
    } else {
      val::ArrayValue<double> dvalue(estimate.get());
      val::ArrayValue<double> duncertainty(uncertainty.get());
      ss << snt::array_to_string(dvalue.get_values(), duncertainty.get_values(), dvalue.get_shape(), fmt);
    }
    return format.format_order(ss.str());
  }

  std::size_t Magnitude::size() const {
    return estimate->get_size();
    //return estimate.size();
  }

  val::Array::ShapeType Magnitude::shape() const {
    return estimate->get_shape();
  }

  /*
   * Add two magnitudes
   */
  Magnitude operator+(const Magnitude& m1, const Magnitude& m2) {
    // z ± Dz = (x ± Dx) + (y ± Dy) -> Dz = Dx + Dy     (average uncertainties)
    val::BaseValue::PointerType Dz = nullptr;
    if (m1.uncertainty && m2.uncertainty)
      Dz = m1.uncertainty->math_add(m2.uncertainty.get());
    else if (m1.uncertainty)
      Dz = m1.uncertainty->clone();
    else if (m2.uncertainty)
      Dz = m2.uncertainty->clone();
    // Array Dz = nostd::sqrt(nostd::pow(m1.uncertainty,2)+nostd::pow(m2.uncertainty,2)); // Gaussian uncertainty propagation
    return Magnitude(m1.estimate->math_add(m2.estimate.get()), std::move(Dz));
    //Array Dz = m1.uncertainty + m2.uncertainty;
    //// Array Dz = nostd::sqrt(nostd::pow(m1.uncertainty,2)+nostd::pow(m2.uncertainty,2)); // Gaussian uncertainty propagation
    //return Magnitude(m1.estimate + m2.estimate, Dz);
  }

  void Magnitude::operator+=(const Magnitude& m) {
    estimate->math_add_equal(m.estimate.get());
    if (uncertainty && m.uncertainty)
      uncertainty->math_add_equal(m.uncertainty.get());
    else if (m.uncertainty)
      uncertainty = m.uncertainty->clone();
    //estimate += m.estimate;
    //uncertainty += m.uncertainty;
    //// uncertainty = nostd::sqrt(nostd::pow(uncertainty,2)+nostd::pow(m.uncertainty,2));
  }

  /*
   * Subtract two magnitudes
   */
  Magnitude operator-(const Magnitude& m1) {
    if (m1.uncertainty)
      return Magnitude(m1.estimate->math_neg(), m1.uncertainty->clone());
    else
      return Magnitude(m1.estimate->math_neg());
    //return Magnitude(-m1.estimate, m1.uncertainty);
  }
  Magnitude operator-(const Magnitude& m1, const Magnitude& m2) {
    // z ± Dz = (x ± Dx) - (y ± Dy) -> Dz = Dx + Dy     (average uncertainties)
    val::BaseValue::PointerType Dz = nullptr;
    if (m1.uncertainty && m2.uncertainty)
      Dz = m1.uncertainty->math_add(m2.uncertainty.get());
    else if (m1.uncertainty)
      Dz = m1.uncertainty->clone();
    else if (m2.uncertainty)
      Dz = m2.uncertainty->clone();
    return Magnitude(m1.estimate->math_sub(m2.estimate.get()), std::move(Dz));
    //return Magnitude(m1.estimate - m2.estimate, m1.uncertainty + m2.uncertainty);
  }

  void Magnitude::operator-=(const Magnitude& m) {
    estimate->math_sub_equal(m.estimate.get());
    if (uncertainty && m.uncertainty)
      uncertainty->math_add_equal(m.uncertainty.get());
    else if (m.uncertainty)
      uncertainty = m.uncertainty->clone();
    //estimate -= m.estimate;
    //uncertainty += m.uncertainty;
  }

  /*
   * Multiply magnitude by another magnitude
   */
  const Magnitude multiply(const Magnitude* m, const Magnitude* n) {
    const val::ArrayValue<double> otherT(n->estimate.get());
    Magnitude nm(m->estimate->math_mul(n->estimate.get()));
    if ((m->uncertainty && n->uncertainty) && (m->uncertainty->any_of() && n->uncertainty->any_of())) {
      val::BaseValue::PointerType maxuncertainty = ((m->estimate->math_add(m->uncertainty.get()))->math_mul((n->estimate->math_add(n->uncertainty.get())).get())->math_sub(nm.estimate.get()))->math_abs();
      val::BaseValue::PointerType minuncertainty = ((m->estimate->math_sub(m->uncertainty.get()))->math_mul((n->estimate->math_sub(n->uncertainty.get())).get())->math_sub(nm.estimate.get()))->math_abs();
      nm.uncertainty = maxuncertainty->math_max(minuncertainty.get());
    } else if ((!m->uncertainty || m->uncertainty->none_of()) && (n->uncertainty && n->uncertainty->any_of())) {
      nm.uncertainty = n->uncertainty->math_mul(m->estimate.get());
    } else if ((m->uncertainty && m->uncertainty->any_of()) && (!n->uncertainty || n->uncertainty->none_of())) {
      nm.uncertainty = m->uncertainty->math_mul(n->estimate.get());
    }
    //Magnitude nm(m->estimate * n->estimate);
    //if (m->uncertainty == 0 && n->uncertainty == 0) {
    //  nm.uncertainty = (m->uncertainty.size() > n->uncertainty.size()) ? m->uncertainty : n->uncertainty;
    //} else if (m->uncertainty == 0 && n->uncertainty != 0) {
    //  nm.uncertainty = n->uncertainty * m->estimate;
    //} else if (m->uncertainty != 0 && n->uncertainty == 0) {
    //  nm.uncertainty = m->uncertainty * n->estimate;
    //} else {
    //  val::BaseValue::PointerType maxuncertainty = nostd::abs((m->estimate + m->uncertainty) * (n->estimate + n->uncertainty) - nm.estimate);
    //  val::BaseValue::PointerType minuncertainty = nostd::abs((m->estimate - m->uncertainty) * (n->estimate - n->uncertainty) - nm.estimate);
    //  nm.uncertainty = nostd::max(maxuncertainty, minuncertainty);
    //}
    return nm;
  }

  Magnitude operator*(const Magnitude& m1, const Magnitude& m2) {
    return multiply(&m1, &m2);
  }

  void Magnitude::operator*=(const Magnitude& m) {
    Magnitude nm = multiply(this, &m);
    estimate = nm.estimate->clone();
    if (nm.uncertainty)
      uncertainty = nm.uncertainty->clone();
    //estimate = nm.estimate;
    //uncertainty = nm.uncertainty;
  }

  /*
   * Divide magnitude by another magnitude
   */
  const Magnitude divide(const Magnitude* m, const Magnitude* n) {
    Magnitude nm(m->estimate->math_div(n->estimate.get()));
    if ((m->uncertainty && n->uncertainty) && (m->uncertainty->any_of() && n->uncertainty->any_of())) {
      val::BaseValue::PointerType maxuncertainty = (((m->estimate->math_add(m->uncertainty.get()))->math_div((n->estimate->math_sub(n->uncertainty.get())).get()))->math_sub(nm.estimate.get()))->math_abs();
      val::BaseValue::PointerType minuncertainty = (((m->estimate->math_sub(m->uncertainty.get()))->math_div((n->estimate->math_add(n->uncertainty.get())).get()))->math_sub(nm.estimate.get()))->math_abs();
      nm.uncertainty = maxuncertainty->math_max(minuncertainty.get());
    } else if ((!m->uncertainty || m->uncertainty->none_of()) && (n->uncertainty && n->uncertainty->any_of())) {
      val::BaseValue::PointerType maxuncertainty = (m->estimate->math_div((n->estimate->math_add(n->uncertainty.get())).get())->math_sub(nm.estimate.get()))->math_abs();
      val::BaseValue::PointerType minuncertainty = (m->estimate->math_div((n->estimate->math_sub(n->uncertainty.get())).get())->math_sub(nm.estimate.get()))->math_abs();
      nm.uncertainty = maxuncertainty->math_max(minuncertainty.get());
    } else if ((m->uncertainty && m->uncertainty->any_of()) && (!n->uncertainty || n->uncertainty->none_of())) {
      nm.uncertainty = m->uncertainty->math_div(n->estimate.get());
    }
    //Magnitude nm(m->estimate / n->estimate);
    //if (m->uncertainty == 0 && n->uncertainty == 0) {
    //  nm.uncertainty = (m->uncertainty.size() > n->uncertainty.size()) ? m->uncertainty : n->uncertainty;
    //} else if (m->uncertainty == 0 && n->uncertainty != 0) {
    //  val::BaseValue::PointerType maxuncertainty = nostd::abs(m->estimate / (n->estimate + n->uncertainty) - nm.estimate);
    //  val::BaseValue::PointerType minuncertainty = nostd::abs(m->estimate / (n->estimate - n->uncertainty) - nm.estimate);
    //  nm.uncertainty = nostd::max(maxuncertainty, minuncertainty);
    //} else if (m->uncertainty != 0 && n->uncertainty == 0) {
    //  nm.uncertainty = m->uncertainty / n->estimate;
    //} else {
    //  val::BaseValue::PointerType maxuncertainty = nostd::abs((m->estimate + m->uncertainty) / (n->estimate - n->uncertainty) - nm.estimate);
    //  val::BaseValue::PointerType minuncertainty = nostd::abs((m->estimate - m->uncertainty) / (n->estimate + n->uncertainty) - nm.estimate);
    //  nm.uncertainty = nostd::max(maxuncertainty, minuncertainty);
    //}
    return nm;
  }

  Magnitude operator/(const Magnitude& m1, const Magnitude& m2) {
    return divide(&m1, &m2);
  }

  void Magnitude::operator/=(const Magnitude& m) {
    Magnitude nm = divide(this, &m);
    estimate = nm.estimate->clone();
    if (nm.uncertainty)
      uncertainty = nm.uncertainty->clone();
    //estimate = nm.estimate;
    //uncertainty = nm.uncertainty;
  }

  void Magnitude::pow(const ExponentVariant& e) {
    double fexp = exponent_to_float(e);
    estimate = estimate->math_pow(fexp);
    //estimate = nostd::pow(estimate, fexp);
  }

  bool Magnitude::operator==(const Magnitude& a) const {
    if (uncertainty && a.uncertainty)
      return (estimate->compare_equal(a.estimate.get())->all_of()) && (uncertainty->compare_equal(a.uncertainty.get())->all_of());
    else if (!uncertainty && !a.uncertainty)
      return (estimate->compare_equal(a.estimate.get())->all_of());
    else
      return false;
    //return (estimate == a.estimate) && (uncertainty == a.uncertainty);
  };

  bool Magnitude::operator!=(const Magnitude& a) const {
    if (uncertainty && a.uncertainty)
      return (estimate->compare_not_equal(a.estimate.get())->any_of()) || (uncertainty->compare_not_equal(a.uncertainty.get())->any_of());
    else if (!uncertainty && !a.uncertainty)
      return (estimate->compare_not_equal(a.estimate.get())->any_of());
    else
      return false;
    //return (estimate != a.estimate) || (uncertainty != a.uncertainty);
  };

  std::ostream& operator<<(std::ostream& os, const Magnitude& m) {
    os << m.to_string();
    return os;
  }

} // namespace snt::puq
