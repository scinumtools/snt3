#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "array.h"
#include "nostd/nostd.h"

#ifdef MAGNITUDE_ARRAYS

namespace puq {

  Array::Array(const ArrayValue& v, const ArrayShape s) : a_shape(s) {
    // calculate array size
    a_size = 1;
    for (size_t sh : a_shape)
      a_size *= sh;
    // set array value
    if (v.size() == 0)
      value = ArrayValue(a_size, 0);
    else if (v.size() != a_size)
      throw ArraySizeException("Number of items in the array does not correspond with the given shape: " + std::to_string(v.size()) + " != " + nostd::to_string(s));
    else
      value = v;
    value.shrink_to_fit();
  }

  inline void mutating_operation(Array& a1, const Array& a2,
                                 void (*func)(MAGNITUDE_PRECISION& v1,
                                              const MAGNITUDE_PRECISION& v2)) {
    if (a1.a_shape == a2.a_shape) // arrays have the same shape
      for (int i = 0; i < a1.size(); i++)
        func(a1.value[i], a2.value[i]);
    else if (a1.size() == 1) // first value is a scalar
      for (int i = 0; i < a2.size(); i++)
        func(a1.value[0], a2.value[i]);
    else if (a2.size() == 1) // second value is a scalar
      for (int i = 0; i < a1.size(); i++)
        func(a1.value[i], a2.value[0]);
    else // arrays have different shapes
      throw ArraySizeException(a1.shape(), a2.shape());
  }

  // static method
  Array Array::const_operation(const Array& a1, const Array& a2,
                               MAGNITUDE_PRECISION (*func)(const MAGNITUDE_PRECISION& v1,
                                                           const MAGNITUDE_PRECISION& v2)) {
    if (a1.a_shape == a2.a_shape) { // arrays have the same shape
      Array a({}, a1.shape());
      for (int i = 0; i < a1.size(); i++)
        a.value[i] = func(a1[i], a2[i]);
      return a;
    } else if (a1.size() == 1) { // first value is a scalar
      Array a({}, a2.shape());
      for (int i = 0; i < a2.size(); i++)
        a.value[i] = func(a1[0], a2[i]);
      return a;
    } else if (a2.size() == 1) { // second value is a scalar
      Array a({}, a1.shape());
      for (int i = 0; i < a1.size(); i++)
        a.value[i] = func(a1[i], a2[0]);
      return a;
    } else { // arrays have different sizes
      throw ArraySizeException(a1.shape(), a2.shape());
    }
  }

  // static method
  Array Array::filled(const MAGNITUDE_PRECISION& v, const size_t& s) {
    ArrayValue av(s, v);
    return Array(av, {s});
  };

  std::string Array::to_string(const UnitFormat& format) const {
    std::stringstream ss;
    if (size() == 1) {
      ss << std::setprecision(format.precision);
      ss << value[0] << std::scientific;
    } else if (size() == 2) {
      ss << std::setprecision(format.precision);
      ss << SYMBOL_ARRAY_START << value[0];
      ss << SYMBOL_ARRAY_SEPARATOR << " " << value[1];
      ss << std::scientific << SYMBOL_ARRAY_END;
    } else if (size() > 2) {
      ss << std::setprecision(format.precision);
      ss << SYMBOL_ARRAY_START << value[0];
      ss << SYMBOL_ARRAY_SEPARATOR << " " << value[1];
      ss << SYMBOL_ARRAY_SEPARATOR << " " << SYMBOL_ARRAY_MORE;
      ss << std::scientific << SYMBOL_ARRAY_END;
    }
    return ss.str();
  }

  size_t Array::size() const {
    return a_size;
  };

  ArrayShape Array::shape() const {
    return a_shape;
  };

  MAGNITUDE_PRECISION Array::operator[](const size_t i) const {
    return value[i];
  }

  Array operator+(const Array& a1, const Array& a2) {
    auto fn = [](const MAGNITUDE_PRECISION& v1, const MAGNITUDE_PRECISION& v2) {
      return v1 + v2;
    };
    return Array::const_operation(a1, a2, fn);
  }

  Array operator-(const Array& a1, const Array& a2) {
    auto fn = [](const MAGNITUDE_PRECISION& v1, const MAGNITUDE_PRECISION& v2) {
      return v1 - v2;
    };
    return Array::const_operation(a1, a2, fn);
  }

  Array operator*(const Array& a1, const Array& a2) {
    auto fn = [](const MAGNITUDE_PRECISION& v1, const MAGNITUDE_PRECISION& v2) {
      return v1 * v2;
    };
    return Array::const_operation(a1, a2, fn);
  }

  Array operator/(const Array& a1, const Array& a2) {
    auto fn = [](const MAGNITUDE_PRECISION& v1, const MAGNITUDE_PRECISION& v2) {
      return v1 / v2;
    };
    return Array::const_operation(a1, a2, fn);
  }

  Array operator-(const Array& a) {
    ArrayValue av;
    for (int i = 0; i < a.size(); i++)
      av.push_back(-a.value[i]);
    return Array(av, a.shape());
  }

  void Array::operator+=(const Array& a) {
    auto fn = [](MAGNITUDE_PRECISION& v1, const MAGNITUDE_PRECISION& v2) {
      v1 += v2;
    };
    mutating_operation(*this, a, fn);
  }

  void Array::operator-=(const Array& a) {
    auto fn = [](MAGNITUDE_PRECISION& v1, const MAGNITUDE_PRECISION& v2) {
      v1 -= v2;
    };
    mutating_operation(*this, a, fn);
  }

  void Array::operator*=(const Array& a) {
    auto fn = [](MAGNITUDE_PRECISION& v1, const MAGNITUDE_PRECISION& v2) {
      v1 *= v2;
    };
    mutating_operation(*this, a, fn);
  }

  void Array::operator/=(const Array& a) {
    auto fn = [](MAGNITUDE_PRECISION& v1, const MAGNITUDE_PRECISION& v2) {
      v1 /= v2;
    };
    mutating_operation(*this, a, fn);
  }

  bool Array::operator==(const Array& a) const {
    if (shape() == a.shape()) { // arrays have the same shape
      for (int i = 0; i < size(); i++)
        if (value[i] != a.value[i])
          return false;
    } else if (size() == 1) { // first value is a scalar
      for (int i = 0; i < a.size(); i++)
        if (value[0] != a.value[i])
          return false;
    } else if (a.size() == 1) { // second value is a scalar
      for (int i = 0; i < size(); i++)
        if (value[i] != a.value[0])
          return false;
    } else { // arrays have different shapes
      throw ArraySizeException(shape(), a.shape());
    }
    return true;
  }

  bool Array::operator!=(const Array& a) const {
    if (shape() == a.shape()) { // arrays have the same shape
      for (int i = 0; i < size(); i++)
        if (value[i] != a.value[i])
          return true;
    } else if (size() == 1) { // first value is a scalar
      for (int i = 0; i < a.size(); i++)
        if (value[0] != a.value[i])
          return true;
    } else if (a.size() == 1) { // second value is a scalar
      for (int i = 0; i < size(); i++)
        if (value[i] != a.value[0])
          return true;
    } else { // arrays have different shapes
      throw ArraySizeException(shape(), a.shape());
    }
    return false;
  }

  std::ostream& operator<<(std::ostream& os, const Array& a) {
    os << a.to_string();
    return os;
  }

  void Array::pow(const EXPONENT_TYPE& e) {
    for (int i = 0; i < size(); i++)
      value[i] = std::pow(value[i], (EXPONENT_REAL_PRECISION)e);
  }

#ifdef EXPONENT_FRACTIONS
  void Array::pow(const EXPONENT_INT_PRECISION& e) {
    for (int i = 0; i < size(); i++)
      value[i] = std::pow(value[i], e);
  }
#endif

} // namespace puq

#endif
