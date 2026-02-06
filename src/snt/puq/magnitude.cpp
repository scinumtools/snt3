#include "magnitude.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef MAGNITUDE_ERRORS

namespace snt::puq {

#if defined(MAGNITUDE_VALUES)

  Magnitude::Magnitude(val::BaseValue::PointerType m) : value(std::move(m)), error(nullptr) {
    if (!value)
      throw std::invalid_argument("Magnitude value cannot be a null pointer.");
  }

  Magnitude::Magnitude(val::BaseValue::PointerType m, val::BaseValue::PointerType e) : value(std::move(m)), error(std::move(e)) {
    if (!value)
      throw std::invalid_argument("Magnitude value cannot be a null pointer.");
    if (error && value->get_size() != error->get_size())
      throw std::invalid_argument("Value and error arrays have different size: " + std::to_string(value->get_size()) + " != " + std::to_string(error->get_size()));
  }

#endif

  /*
   * Convert absolute and relative (in %) errors
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
    if (error == nullptr || !format.display_error()) {
      ss << value->to_string(fmt);
    } else {
      val::ArrayValue<double> dvalue(value.get());
      val::ArrayValue<double> derror(error.get());
      ss << snt::array_to_string(dvalue.get_values(), derror.get_values(), dvalue.get_shape(), fmt);
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
    // z ± Dz = (x ± Dx) + (y ± Dy) -> Dz = Dx + Dy     (average errors)
    val::BaseValue::PointerType Dz = nullptr;
    if (m1.error && m2.error)
      Dz = m1.error->math_add(m2.error.get());
    else if (m1.error)
      Dz = m1.error->clone();
    else if (m2.error)
      Dz = m2.error->clone();
    // Array Dz = nostd::sqrt(nostd::pow(m1.error,2)+nostd::pow(m2.error,2)); // Gaussian error propagation
    return Magnitude(m1.value->math_add(m2.value.get()), std::move(Dz));
#else
    // z ± Dz = (x ± Dx) + (y ± Dy) -> Dz = Dx + Dy     (average errors)
    Array Dz = m1.error + m2.error;
    // Array Dz = nostd::sqrt(nostd::pow(m1.error,2)+nostd::pow(m2.error,2)); // Gaussian error propagation
    return Magnitude(m1.value + m2.value, Dz);
#endif
  }

  void Magnitude::operator+=(const Magnitude& m) {
#ifdef MAGNITUDE_VALUES
    value->math_add_equal(m.value.get());
    if (error && m.error)
      error->math_add_equal(m.error.get());
    else if (m.error)
      error = m.error->clone();
#else
    value += m.value;
    error += m.error;
    // error = nostd::sqrt(nostd::pow(error,2)+nostd::pow(m.error,2));
#endif
  }

  /*
   * Subtract two magnitudes
   */
  Magnitude operator-(const Magnitude& m1) {
#ifdef MAGNITUDE_VALUES
    if (m1.error)
      return Magnitude(m1.value->math_neg(), m1.error->clone());
    else
      return Magnitude(m1.value->math_neg());
#else
    return Magnitude(-m1.value, m1.error);
#endif
  }
  Magnitude operator-(const Magnitude& m1, const Magnitude& m2) {
    // z ± Dz = (x ± Dx) - (y ± Dy) -> Dz = Dx + Dy     (average errors)
#ifdef MAGNITUDE_VALUES
    val::BaseValue::PointerType Dz = nullptr;
    if (m1.error && m2.error)
      Dz = m1.error->math_add(m2.error.get());
    else if (m1.error)
      Dz = m1.error->clone();
    else if (m2.error)
      Dz = m2.error->clone();
    return Magnitude(m1.value->math_sub(m2.value.get()), std::move(Dz));
#else
    return Magnitude(m1.value - m2.value, m1.error + m2.error);
#endif
  }

  void Magnitude::operator-=(const Magnitude& m) {
#ifdef MAGNITUDE_VALUES
    value->math_sub_equal(m.value.get());
    if (error && m.error)
      error->math_add_equal(m.error.get());
    else if (m.error)
      error = m.error->clone();
#else
    value -= m.value;
    error += m.error;
#endif
  }

  /*
   * Multiply magnitude by another magnitude
   */
  const Magnitude multiply(const Magnitude* m, const Magnitude* n) {
#ifdef MAGNITUDE_VALUES
    const val::ArrayValue<double> otherT(n->value.get());
    Magnitude nm(m->value->math_mul(n->value.get()));
    if ((m->error && n->error) && (m->error->any_of() && n->error->any_of())) {
      MAGNITUDE_VALUE maxerror = ((m->value->math_add(m->error.get()))->math_mul((n->value->math_add(n->error.get())).get())->math_sub(nm.value.get()))->math_abs();
      MAGNITUDE_VALUE minerror = ((m->value->math_sub(m->error.get()))->math_mul((n->value->math_sub(n->error.get())).get())->math_sub(nm.value.get()))->math_abs();
      nm.error = maxerror->math_max(minerror.get());
    } else if ((!m->error || m->error->none_of()) && (n->error && n->error->any_of())) {
      nm.error = n->error->math_mul(m->value.get());
    } else if ((m->error && m->error->any_of()) && (!n->error || n->error->none_of())) {
      nm.error = m->error->math_mul(n->value.get());
    }
#else
    Magnitude nm(m->value * n->value);
    if (m->error == 0 && n->error == 0) {
      nm.error = (m->error.size() > n->error.size()) ? m->error : n->error;
    } else if (m->error == 0 && n->error != 0) {
      nm.error = n->error * m->value;
    } else if (m->error != 0 && n->error == 0) {
      nm.error = m->error * n->value;
    } else {
      MAGNITUDE_VALUE maxerror = nostd::abs((m->value + m->error) * (n->value + n->error) - nm.value);
      MAGNITUDE_VALUE minerror = nostd::abs((m->value - m->error) * (n->value - n->error) - nm.value);
      nm.error = nostd::max(maxerror, minerror);
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
    if (nm.error)
      error = nm.error->clone();
#else
    value = nm.value;
    error = nm.error;
#endif
  }

  /*
   * Divide magnitude by another magnitude
   */
  const Magnitude divide(const Magnitude* m, const Magnitude* n) {
#ifdef MAGNITUDE_VALUES
    Magnitude nm(m->value->math_div(n->value.get()));
    if ((m->error && n->error) && (m->error->any_of() && n->error->any_of())) {
      MAGNITUDE_VALUE maxerror = (((m->value->math_add(m->error.get()))->math_div((n->value->math_sub(n->error.get())).get()))->math_sub(nm.value.get()))->math_abs();
      MAGNITUDE_VALUE minerror = (((m->value->math_sub(m->error.get()))->math_div((n->value->math_add(n->error.get())).get()))->math_sub(nm.value.get()))->math_abs();
      nm.error = maxerror->math_max(minerror.get());
    } else if ((!m->error || m->error->none_of()) && (n->error && n->error->any_of())) {
      MAGNITUDE_VALUE maxerror = (m->value->math_div((n->value->math_add(n->error.get())).get())->math_sub(nm.value.get()))->math_abs();
      MAGNITUDE_VALUE minerror = (m->value->math_div((n->value->math_sub(n->error.get())).get())->math_sub(nm.value.get()))->math_abs();
      nm.error = maxerror->math_max(minerror.get());
    } else if ((m->error && m->error->any_of()) && (!n->error || n->error->none_of())) {
      nm.error = m->error->math_div(n->value.get());
    }
#else
    Magnitude nm(m->value / n->value);
    if (m->error == 0 && n->error == 0) {
      nm.error = (m->error.size() > n->error.size()) ? m->error : n->error;
    } else if (m->error == 0 && n->error != 0) {
      MAGNITUDE_VALUE maxerror = nostd::abs(m->value / (n->value + n->error) - nm.value);
      MAGNITUDE_VALUE minerror = nostd::abs(m->value / (n->value - n->error) - nm.value);
      nm.error = nostd::max(maxerror, minerror);
    } else if (m->error != 0 && n->error == 0) {
      nm.error = m->error / n->value;
    } else {
      MAGNITUDE_VALUE maxerror = nostd::abs((m->value + m->error) / (n->value - n->error) - nm.value);
      MAGNITUDE_VALUE minerror = nostd::abs((m->value - m->error) / (n->value + n->error) - nm.value);
      nm.error = nostd::max(maxerror, minerror);
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
    if (nm.error)
      error = nm.error->clone();
#else
    value = nm.value;
    error = nm.error;
#endif
  }

  void Magnitude::pow(const EXPONENT_TYPE& e) {
#ifdef MAGNITUDE_VALUES
    value = value->math_pow((ExponentFloat)e);
#else
    value = nostd::pow(value, (ExponentFloat)e);
#endif
  }

  bool Magnitude::operator==(const Magnitude& a) const {
#ifdef MAGNITUDE_VALUES
    if (error && a.error)
      return (value->compare_equal(a.value.get())->all_of()) && (error->compare_equal(a.error.get())->all_of());
    else if (!error && !a.error)
      return (value->compare_equal(a.value.get())->all_of());
    else
      return false;
#else
    return (value == a.value) && (error == a.error);
#endif
  };

  bool Magnitude::operator!=(const Magnitude& a) const {
#ifdef MAGNITUDE_VALUES
    if (error && a.error)
      return (value->compare_not_equal(a.value.get())->any_of()) || (error->compare_not_equal(a.error.get())->any_of());
    else if (!error && !a.error)
      return (value->compare_not_equal(a.value.get())->any_of());
    else
      return false;
#else
    return (value != a.value) || (error != a.error);
#endif
  };

  std::ostream& operator<<(std::ostream& os, const Magnitude& m) {
    os << m.to_string();
    return os;
  }

} // namespace snt::puq

#endif
