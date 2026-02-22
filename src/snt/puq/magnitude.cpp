#include <snt/puq/magnitude.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef MAGNITUDE_UNCERTAINTIES

namespace snt::puq {

#if defined(MAGNITUDE_VALUES)

  Magnitude::Magnitude(val::BaseValue::PointerType m) : value(std::move(m)), uncertainty(nullptr) {
    if (!value)
      throw std::invalid_argument("Magnitude value cannot be a null pointer.");
  }

  Magnitude::Magnitude(val::BaseValue::PointerType m, val::BaseValue::PointerType e) : value(std::move(m)), uncertainty(std::move(e)) {
    if (!value)
      throw std::invalid_argument("Magnitude value cannot be a null pointer.");
    if (uncertainty && value->get_size() != uncertainty->get_size())
      throw std::invalid_argument("Value and uncertainty arrays have different size: " + std::to_string(value->get_size()) + " != " + std::to_string(uncertainty->get_size()));
  }

#endif

  /*
   * Convert absolute and relative (in %) uncertainties
   */
  MagnitudeFloat Magnitude::abs_to_rel(const MagnitudeFloat& v, const MagnitudeFloat& e) {
    return 100 * e / v;
  };

  MagnitudeFloat Magnitude::rel_to_abs(const MagnitudeFloat& v, const MagnitudeFloat& e) {
    return v * e / 100;
  };

#if defined(MAGNITUDE_VALUES)

  // DEBUG: need to implement properly
  val::BaseValue::PointerType Magnitude::abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
    return e->math_div(v.get())->math_mul(std::make_unique<val::ArrayValue<double>>(100).get());
  };

  val::BaseValue::PointerType Magnitude::rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
    return v->math_mul(e.get())->math_div(std::make_unique<val::ArrayValue<double>>(100).get());
  };

#endif

  /*
   * Return a string representation of a magnitude
   */
  std::string Magnitude::to_string(const UnitFormat& format) const {
    std::stringstream ss;
    snt::StringFormatType fmt;
    fmt.valuePrecision = format.precision;
    if (uncertainty == nullptr || !format.display_uncertainty()) {
      ss << value->to_string(fmt);
    } else {
      val::ArrayValue<double> dvalue(value.get());
      val::ArrayValue<double> duncertainty(uncertainty.get());
      ss << snt::array_to_string(dvalue.get_values(), duncertainty.get_values(), dvalue.get_shape(), fmt);
    }
    return format.format_order(ss.str());
  }

  std::size_t Magnitude::size() const {
#if defined(MAGNITUDE_VALUES)
    return value->get_size();
#else
    return value.size();
#endif
  }

#if defined(MAGNITUDE_VALUES)
  val::Array::ShapeType Magnitude::shape() const {
    return value->get_shape();
  }
#endif

  /*
   * Add two magnitudes
   */
  Magnitude operator+(const Magnitude& m1, const Magnitude& m2) {
#ifdef MAGNITUDE_VALUES
    // z ± Dz = (x ± Dx) + (y ± Dy) -> Dz = Dx + Dy     (average uncertainties)
    val::BaseValue::PointerType Dz = nullptr;
    if (m1.uncertainty && m2.uncertainty)
      Dz = m1.uncertainty->math_add(m2.uncertainty.get());
    else if (m1.uncertainty)
      Dz = m1.uncertainty->clone();
    else if (m2.uncertainty)
      Dz = m2.uncertainty->clone();
    // Array Dz = nostd::sqrt(nostd::pow(m1.uncertainty,2)+nostd::pow(m2.uncertainty,2)); // Gaussian uncertainty propagation
    return Magnitude(m1.value->math_add(m2.value.get()), std::move(Dz));
#else
    // z ± Dz = (x ± Dx) + (y ± Dy) -> Dz = Dx + Dy     (average uncertainties)
    Array Dz = m1.uncertainty + m2.uncertainty;
    // Array Dz = nostd::sqrt(nostd::pow(m1.uncertainty,2)+nostd::pow(m2.uncertainty,2)); // Gaussian uncertainty propagation
    return Magnitude(m1.value + m2.value, Dz);
#endif
  }

  void Magnitude::operator+=(const Magnitude& m) {
#ifdef MAGNITUDE_VALUES
    value->math_add_equal(m.value.get());
    if (uncertainty && m.uncertainty)
      uncertainty->math_add_equal(m.uncertainty.get());
    else if (m.uncertainty)
      uncertainty = m.uncertainty->clone();
#else
    value += m.value;
    uncertainty += m.uncertainty;
    // uncertainty = nostd::sqrt(nostd::pow(uncertainty,2)+nostd::pow(m.uncertainty,2));
#endif
  }

  /*
   * Subtract two magnitudes
   */
  Magnitude operator-(const Magnitude& m1) {
#ifdef MAGNITUDE_VALUES
    if (m1.uncertainty)
      return Magnitude(m1.value->math_neg(), m1.uncertainty->clone());
    else
      return Magnitude(m1.value->math_neg());
#else
    return Magnitude(-m1.value, m1.uncertainty);
#endif
  }
  Magnitude operator-(const Magnitude& m1, const Magnitude& m2) {
    // z ± Dz = (x ± Dx) - (y ± Dy) -> Dz = Dx + Dy     (average uncertainties)
#ifdef MAGNITUDE_VALUES
    val::BaseValue::PointerType Dz = nullptr;
    if (m1.uncertainty && m2.uncertainty)
      Dz = m1.uncertainty->math_add(m2.uncertainty.get());
    else if (m1.uncertainty)
      Dz = m1.uncertainty->clone();
    else if (m2.uncertainty)
      Dz = m2.uncertainty->clone();
    return Magnitude(m1.value->math_sub(m2.value.get()), std::move(Dz));
#else
    return Magnitude(m1.value - m2.value, m1.uncertainty + m2.uncertainty);
#endif
  }

  void Magnitude::operator-=(const Magnitude& m) {
#ifdef MAGNITUDE_VALUES
    value->math_sub_equal(m.value.get());
    if (uncertainty && m.uncertainty)
      uncertainty->math_add_equal(m.uncertainty.get());
    else if (m.uncertainty)
      uncertainty = m.uncertainty->clone();
#else
    value -= m.value;
    uncertainty += m.uncertainty;
#endif
  }

  /*
   * Multiply magnitude by another magnitude
   */
  const Magnitude multiply(const Magnitude* m, const Magnitude* n) {
#ifdef MAGNITUDE_VALUES
    const val::ArrayValue<double> otherT(n->value.get());
    Magnitude nm(m->value->math_mul(n->value.get()));
    if ((m->uncertainty && n->uncertainty) && (m->uncertainty->any_of() && n->uncertainty->any_of())) {
      MAGNITUDE_VALUE maxuncertainty = ((m->value->math_add(m->uncertainty.get()))->math_mul((n->value->math_add(n->uncertainty.get())).get())->math_sub(nm.value.get()))->math_abs();
      MAGNITUDE_VALUE minuncertainty = ((m->value->math_sub(m->uncertainty.get()))->math_mul((n->value->math_sub(n->uncertainty.get())).get())->math_sub(nm.value.get()))->math_abs();
      nm.uncertainty = maxuncertainty->math_max(minuncertainty.get());
    } else if ((!m->uncertainty || m->uncertainty->none_of()) && (n->uncertainty && n->uncertainty->any_of())) {
      nm.uncertainty = n->uncertainty->math_mul(m->value.get());
    } else if ((m->uncertainty && m->uncertainty->any_of()) && (!n->uncertainty || n->uncertainty->none_of())) {
      nm.uncertainty = m->uncertainty->math_mul(n->value.get());
    }
#else
    Magnitude nm(m->value * n->value);
    if (m->uncertainty == 0 && n->uncertainty == 0) {
      nm.uncertainty = (m->uncertainty.size() > n->uncertainty.size()) ? m->uncertainty : n->uncertainty;
    } else if (m->uncertainty == 0 && n->uncertainty != 0) {
      nm.uncertainty = n->uncertainty * m->value;
    } else if (m->uncertainty != 0 && n->uncertainty == 0) {
      nm.uncertainty = m->uncertainty * n->value;
    } else {
      MAGNITUDE_VALUE maxuncertainty = nostd::abs((m->value + m->uncertainty) * (n->value + n->uncertainty) - nm.value);
      MAGNITUDE_VALUE minuncertainty = nostd::abs((m->value - m->uncertainty) * (n->value - n->uncertainty) - nm.value);
      nm.uncertainty = nostd::max(maxuncertainty, minuncertainty);
    }
#endif
    return nm;
  }

  Magnitude operator*(const Magnitude& m1, const Magnitude& m2) {
    return multiply(&m1, &m2);
  }

  void Magnitude::operator*=(const Magnitude& m) {
    Magnitude nm = multiply(this, &m);
#ifdef MAGNITUDE_VALUES
    value = nm.value->clone();
    if (nm.uncertainty)
      uncertainty = nm.uncertainty->clone();
#else
    value = nm.value;
    uncertainty = nm.uncertainty;
#endif
  }

  /*
   * Divide magnitude by another magnitude
   */
  const Magnitude divide(const Magnitude* m, const Magnitude* n) {
#ifdef MAGNITUDE_VALUES
    Magnitude nm(m->value->math_div(n->value.get()));
    if ((m->uncertainty && n->uncertainty) && (m->uncertainty->any_of() && n->uncertainty->any_of())) {
      MAGNITUDE_VALUE maxuncertainty = (((m->value->math_add(m->uncertainty.get()))->math_div((n->value->math_sub(n->uncertainty.get())).get()))->math_sub(nm.value.get()))->math_abs();
      MAGNITUDE_VALUE minuncertainty = (((m->value->math_sub(m->uncertainty.get()))->math_div((n->value->math_add(n->uncertainty.get())).get()))->math_sub(nm.value.get()))->math_abs();
      nm.uncertainty = maxuncertainty->math_max(minuncertainty.get());
    } else if ((!m->uncertainty || m->uncertainty->none_of()) && (n->uncertainty && n->uncertainty->any_of())) {
      MAGNITUDE_VALUE maxuncertainty = (m->value->math_div((n->value->math_add(n->uncertainty.get())).get())->math_sub(nm.value.get()))->math_abs();
      MAGNITUDE_VALUE minuncertainty = (m->value->math_div((n->value->math_sub(n->uncertainty.get())).get())->math_sub(nm.value.get()))->math_abs();
      nm.uncertainty = maxuncertainty->math_max(minuncertainty.get());
    } else if ((m->uncertainty && m->uncertainty->any_of()) && (!n->uncertainty || n->uncertainty->none_of())) {
      nm.uncertainty = m->uncertainty->math_div(n->value.get());
    }
#else
    Magnitude nm(m->value / n->value);
    if (m->uncertainty == 0 && n->uncertainty == 0) {
      nm.uncertainty = (m->uncertainty.size() > n->uncertainty.size()) ? m->uncertainty : n->uncertainty;
    } else if (m->uncertainty == 0 && n->uncertainty != 0) {
      MAGNITUDE_VALUE maxuncertainty = nostd::abs(m->value / (n->value + n->uncertainty) - nm.value);
      MAGNITUDE_VALUE minuncertainty = nostd::abs(m->value / (n->value - n->uncertainty) - nm.value);
      nm.uncertainty = nostd::max(maxuncertainty, minuncertainty);
    } else if (m->uncertainty != 0 && n->uncertainty == 0) {
      nm.uncertainty = m->uncertainty / n->value;
    } else {
      MAGNITUDE_VALUE maxuncertainty = nostd::abs((m->value + m->uncertainty) / (n->value - n->uncertainty) - nm.value);
      MAGNITUDE_VALUE minuncertainty = nostd::abs((m->value - m->uncertainty) / (n->value + n->uncertainty) - nm.value);
      nm.uncertainty = nostd::max(maxuncertainty, minuncertainty);
    }
#endif
    return nm;
  }

  Magnitude operator/(const Magnitude& m1, const Magnitude& m2) {
    return divide(&m1, &m2);
  }

  void Magnitude::operator/=(const Magnitude& m) {
    Magnitude nm = divide(this, &m);
#ifdef MAGNITUDE_VALUES
    value = nm.value->clone();
    if (nm.uncertainty)
      uncertainty = nm.uncertainty->clone();
#else
    value = nm.value;
    uncertainty = nm.uncertainty;
#endif
  }

  void Magnitude::pow(const ExponentVariant& e) {
    ExponentFloat fexp = exponent_to_float(e);
#ifdef MAGNITUDE_VALUES
    value = value->math_pow(fexp);
#else
    value = nostd::pow(value, fexp);
#endif
  }

  bool Magnitude::operator==(const Magnitude& a) const {
#ifdef MAGNITUDE_VALUES
    if (uncertainty && a.uncertainty)
      return (value->compare_equal(a.value.get())->all_of()) && (uncertainty->compare_equal(a.uncertainty.get())->all_of());
    else if (!uncertainty && !a.uncertainty)
      return (value->compare_equal(a.value.get())->all_of());
    else
      return false;
#else
    return (value == a.value) && (uncertainty == a.uncertainty);
#endif
  };

  bool Magnitude::operator!=(const Magnitude& a) const {
#ifdef MAGNITUDE_VALUES
    if (uncertainty && a.uncertainty)
      return (value->compare_not_equal(a.value.get())->any_of()) || (uncertainty->compare_not_equal(a.uncertainty.get())->any_of());
    else if (!uncertainty && !a.uncertainty)
      return (value->compare_not_equal(a.value.get())->any_of());
    else
      return false;
#else
    return (value != a.value) || (uncertainty != a.uncertainty);
#endif
  };

  std::ostream& operator<<(std::ostream& os, const Magnitude& m) {
    os << m.to_string();
    return os;
  }

} // namespace snt::puq

#endif
