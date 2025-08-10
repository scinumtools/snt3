#include <sstream>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "magnitude.h"
#include "nostd/nostd.h"

#ifdef MAGNITUDE_ERRORS

namespace puq {
  
#if defined(MAGNITUDE_ARRAYS)

  Magnitude::Magnitude(const Array& m): value(m), error(0) {
    ArrayValue av;
    for (int i=0; i<m.size(); i++)
      av.push_back(0);
    error = Array(av,m.shape());
  }

  Magnitude::Magnitude(const Array& m, const Array& e): value(m), error(e) {
    if (m.size()!=e.size()) 
      throw std::invalid_argument("Value and error arrays have different size: "+std::to_string(m.size())+" != "+std::to_string(e.size()));
  }

#elif defined(MAGNITUDE_VALUES)

  Magnitude::Magnitude(val::BaseValue::PointerType m): value(std::move(m)), error(nullptr) {
    if (!value)
      throw std::invalid_argument("Magnitude value cannot be a null pointer.");
  }

  Magnitude::Magnitude(val::BaseValue::PointerType m, val::BaseValue::PointerType e): value(std::move(m)), error(std::move(e)) {
    if (!value)
      throw std::invalid_argument("Magnitude value cannot be a null pointer.");
    if (error && value->get_size()!=error->get_size()) 
      throw std::invalid_argument("Value and error arrays have different size: "+std::to_string(value->get_size())+" != "+std::to_string(error->get_size()));
  }

#endif

  /*
   * Convert absolute and relative (in %) errors
   */
  MAGNITUDE_PRECISION Magnitude::abs_to_rel(const MAGNITUDE_PRECISION& v, const MAGNITUDE_PRECISION& e) {
    return 100*e/v;
  };

  MAGNITUDE_PRECISION Magnitude::rel_to_abs(const MAGNITUDE_PRECISION& v, const MAGNITUDE_PRECISION& e) {
    return v*e/100;
  };

#if defined(MAGNITUDE_ARRAYS)

  Array Magnitude::abs_to_rel(const Array& v, const Array& e) {
    return Array::filled(100, v.size())*e/v;
  };

  Array Magnitude::rel_to_abs(const Array& v, const Array& e) {
    return v*e/Array::filled(100, v.size());
  };

#elif defined(MAGNITUDE_VALUES)
  
  // DEBUG: need to implement properly
  val::BaseValue::PointerType Magnitude::abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
    return e->math_div(v.get())->math_mul(100);
  };

  val::BaseValue::PointerType Magnitude::rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
    return v->math_mul(e.get())->math_div(100);
  };

#endif

  /*
   * Return a string representation of a magnitude
   */
  std::string _to_string(const MAGNITUDE_PRECISION& value, const MAGNITUDE_PRECISION& error, const UnitFormat& format) {
    std::stringstream ss;
    int exp_val  = std::floor(std::log10(std::abs(value)));
    if (error==0 || !format.display_error()) {
      ss << std::setprecision(format.precision);
      ss << value << std::scientific;
    } else {
      int exp_err  = std::floor(std::log10(error));
      int exp_diff = std::abs(exp_val-exp_err)+1;
      MAGNITUDE_PRECISION val_mag = value*std::pow(10,-exp_val);
      int val_err  = std::round(error*std::pow(10,1-exp_err));
      ss << std::fixed << std::setprecision(exp_diff) << val_mag;
      ss << "(" << std::setw(2) << std::setfill(' ') << val_err << ")";
      if (exp_val != 0) {
        ss << (exp_val >= 0 ? "e+" : "e-")
           << std::setw(2) << std::setfill('0') << std::abs(exp_val);
        ss << std::setfill(' '); 
      }
    }
    return ss.str();  
  }
  std::string Magnitude::to_string(const UnitFormat& format) const {
    std::stringstream ss;
    if (error==0 || !format.display_error()) {
#if defined(MAGNITUDE_ARRAYS)
      ss << value.to_string(format);
#elif defined(MAGNITUDE_VALUES)
      val::ArrayValue<double> dvalue(value.get());
      if (dvalue.get_size()==1) {
	ss << std::setprecision(format.precision);
	ss << dvalue.get_value(0) << std::scientific;
      } else if (dvalue.get_size()==2) {
	ss << std::setprecision(format.precision);
	ss << SYMBOL_ARRAY_START << dvalue.get_value(0);
	ss << SYMBOL_ARRAY_SEPARATOR << " " << dvalue.get_value(1);
	ss << std::scientific << SYMBOL_ARRAY_END;
      } else if (dvalue.get_size()>2) {
	ss << std::setprecision(format.precision);
	ss << SYMBOL_ARRAY_START << dvalue.get_value(0);
	ss << SYMBOL_ARRAY_SEPARATOR << " " << dvalue.get_value(1);
	ss << SYMBOL_ARRAY_SEPARATOR << " " << SYMBOL_ARRAY_MORE;
	ss << std::scientific << SYMBOL_ARRAY_END;
      }
#else
      ss << std::setprecision(format.precision);
      ss << value << std::scientific;
#endif
    }
    else {
#if defined(MAGNITUDE_ARRAYS)
      if (value.size()==1)
	ss << _to_string(value[0], error[0], format);
      else if (value.size()==2) {
	ss << SYMBOL_ARRAY_START << _to_string(value[0], error[0], format);
	ss << SYMBOL_ARRAY_SEPARATOR << " " << _to_string(value[1], error[1], format);
	ss << SYMBOL_ARRAY_END;
      } else {
	ss << SYMBOL_ARRAY_START << _to_string(value[0], error[0], format); 
	ss << SYMBOL_ARRAY_SEPARATOR << " " << _to_string(value[1], error[1], format);
	ss << SYMBOL_ARRAY_SEPARATOR << " " << SYMBOL_ARRAY_MORE << SYMBOL_ARRAY_END;
      }
#elif defined(MAGNITUDE_VALUES)
      val::ArrayValue<double> dvalue(value.get());
      val::ArrayValue<double> derror(error.get());
      if (dvalue.get_size()==1)
	ss << _to_string(dvalue.get_value(0), derror.get_value(0), format);
      else if (dvalue.get_size()==2) {
	ss << SYMBOL_ARRAY_START << _to_string(dvalue.get_value(0), derror.get_value(0), format);
	ss << SYMBOL_ARRAY_SEPARATOR << " " << _to_string(dvalue.get_value(1), derror.get_value(1), format);
	ss << SYMBOL_ARRAY_END;
      } else {
	ss << SYMBOL_ARRAY_START << _to_string(dvalue.get_value(0), derror.get_value(0), format); 
	ss << SYMBOL_ARRAY_SEPARATOR << " " << _to_string(dvalue.get_value(1), derror.get_value(1), format);
	ss << SYMBOL_ARRAY_SEPARATOR << " " << SYMBOL_ARRAY_MORE << SYMBOL_ARRAY_END;
      }
#else
      ss << _to_string(value, error, format);
#endif
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

#if defined(MAGNITUDE_ARRAYS)
  ArrayShape Magnitude::shape() const {
    return value.shape();
  }
#elif defined(MAGNITUDE_VALUES)
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
    //Array Dz = nostd::sqrt(nostd::pow(m1.error,2)+nostd::pow(m2.error,2)); // Gaussian error propagation
    return Magnitude(m1.value->math_add(m2.value.get()), std::move(Dz));
#else
    // z ± Dz = (x ± Dx) + (y ± Dy) -> Dz = Dx + Dy     (average errors)
    Array Dz = m1.error+m2.error;
    //Array Dz = nostd::sqrt(nostd::pow(m1.error,2)+nostd::pow(m2.error,2)); // Gaussian error propagation
    return Magnitude(m1.value+m2.value, Dz);
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
    //error = nostd::sqrt(nostd::pow(error,2)+nostd::pow(m.error,2));
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
    return Magnitude(m1.value-m2.value, m1.error+m2.error);
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
    if (m->error==0 && n->error==0) {
      nm.error = (m->error.size()>n->error.size()) ? m->error : n->error;
    } else if (m->error==0 && n->error!=0) {
      nm.error = n->error * m->value;
    } else if (m->error!=0 && n->error==0) {
      nm.error = m->error * n->value;    
    } else {
      MAGNITUDE_VALUE maxerror = nostd::abs((m->value+m->error)*(n->value+n->error) - nm.value);
      MAGNITUDE_VALUE minerror = nostd::abs((m->value-m->error)*(n->value-n->error) - nm.value);
      nm.error = nostd::max(maxerror,minerror);
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
    if (m->error==0 && n->error==0) {
      nm.error = (m->error.size()>n->error.size()) ? m->error : n->error;
    } else if (m->error==0 && n->error!=0) {
      MAGNITUDE_VALUE maxerror = nostd::abs(m->value/(n->value+n->error) - nm.value);
      MAGNITUDE_VALUE minerror = nostd::abs(m->value/(n->value-n->error) - nm.value);
      nm.error = nostd::max(maxerror,minerror);
    } else if (m->error!=0 && n->error==0) {
      nm.error = m->error / n->value;    
    } else {
      MAGNITUDE_VALUE maxerror = nostd::abs((m->value+m->error)/(n->value-n->error) - nm.value);
      MAGNITUDE_VALUE minerror = nostd::abs((m->value-m->error)/(n->value+n->error) - nm.value);
      nm.error = nostd::max(maxerror,minerror);
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
    value = value->math_pow((EXPONENT_REAL_PRECISION)e);
#else
    value = nostd::pow(value, (EXPONENT_REAL_PRECISION)e);
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
    return (value==a.value) && (error==a.error);
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
    return (value!=a.value) || (error!=a.error);
#endif
  };

  std::ostream& operator<<(std::ostream& os, const Magnitude& m) {
    os << m.to_string();
    return os;
  }

}
 
#endif
