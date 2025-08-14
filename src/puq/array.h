#ifndef PUQ_ARRAY_H
#define PUQ_ARRAY_H

#include <array>
#include <iostream>
#include <vector>

#include "exponent.h"
#include "settings.h"

#ifdef MAGNITUDE_ARRAYS

namespace puq {

  typedef std::vector<MAGNITUDE_PRECISION> ArrayValue;
  typedef std::vector<size_t> ArrayShape;
  class Array {
  public:
    ArrayValue value;
    ArrayShape a_shape;
    size_t a_size;
    Array(const ArrayValue& v, const ArrayShape s);
    Array(const std::vector<MAGNITUDE_PRECISION>& v) : Array(v, {v.size()}) {};
    Array(const MAGNITUDE_PRECISION& v) : Array({v}, {1}) {};
    static Array const_operation(const Array& a1, const Array& a2,
                                 MAGNITUDE_PRECISION (*func)(const MAGNITUDE_PRECISION& v1,
                                                             const MAGNITUDE_PRECISION& v2));
    static Array filled(const MAGNITUDE_PRECISION& v, const size_t& s);
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    size_t size() const;
    ArrayShape shape() const;
    friend Array operator+(const Array& a1, const Array& a2);
    friend Array operator-(const Array& a1, const Array& a2);
    friend Array operator*(const Array& a1, const Array& a2);
    friend Array operator/(const Array& a1, const Array& a2);
    friend Array operator-(const Array& a);
    friend std::ostream& operator<<(std::ostream& os, const Array& a);
    void operator+=(const Array& a);
    void operator-=(const Array& a);
    void operator*=(const Array& a);
    void operator/=(const Array& a);
    bool operator==(const Array& a) const;
    bool operator!=(const Array& a) const;
    MAGNITUDE_PRECISION operator[](const size_t i) const;
    void pow(const EXPONENT_TYPE& e);
#ifdef EXPONENT_FRACTIONS
    void pow(const EXPONENT_INT_PRECISION& e);
#endif
  };

  class ArraySizeException : public std::exception {
  private:
    std::string message;

  public:
    ArraySizeException(std::string m) : message(m) {}
    ArraySizeException(const ArrayShape s1, const ArrayShape s2) {
      std::stringstream ss;
      ss << "Arrays have different shapes: ";
      ss << "[";
      for (int i = 0; i < s1.size(); i++) {
        if (i > 0)
          ss << ",";
        ss << std::to_string(s1[i]);
      }
      ss << "] != [";
      for (int i = 0; i < s2.size(); i++) {
        if (i > 0)
          ss << ",";
        ss << std::to_string(s2[i]);
      }
      ss << "]";
      message = ss.str();
    }
    const char* what() const noexcept override {
      return message.c_str();
    }
  };

} // namespace puq

#endif

#endif // PUQ_ARRAY_H
