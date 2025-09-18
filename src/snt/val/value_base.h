#ifndef VAL_BASE_VALUE_H
#define VAL_BASE_VALUE_H

#include "../settings.h"
#include "array.h"
#include "datatypes.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace snt::val {

  // Forward declaration
  template <typename T>
  class ArrayValue;

  // Define base value class
  class BaseValue {
  protected:
    Array::ShapeType shape;
    DataType dtype;

  public:
    typedef std::unique_ptr<BaseValue> PointerType;
    BaseValue(DataType dt, Array::ShapeType sh) : dtype(dt), shape(sh) {};
    virtual ~BaseValue() = default;
    virtual void print() = 0;
    virtual std::string to_string(const snt::StringFormatType& format = snt::StringFormatType()) const = 0;
    Array::ShapeType get_shape() const { return shape; };
    DataType get_dtype() const { return dtype; };
    virtual size_t get_size() const = 0;
    virtual BaseValue::PointerType clone() const = 0;
    virtual BaseValue::PointerType cast_as(DataType dt) const = 0;
    virtual BaseValue::PointerType slice(const Array::RangeType& slice) = 0;
    // Arithmetic operators
    virtual BaseValue::PointerType math_add(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual void math_add_equal(const BaseValue* other) { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_sub(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual void math_sub_equal(const BaseValue* other) { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_inv() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_mul(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual void math_mul_equal(const BaseValue* other) { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_div(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual void math_div_equal(const BaseValue* other) { throw std::logic_error("method is not implemented for this type"); };
    // Group operators
    virtual BaseValue::PointerType math_sin() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_cos() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_tan() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_log() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_log10() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_sqrt() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_cbrt() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_exp() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_floor() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_ceil() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_abs() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_neg() const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_pow(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_pow(const double exp) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_max(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType math_min(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    // Comparison operators
    virtual BaseValue::PointerType compare_equal(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType compare_not_equal(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType compare_less(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType compare_greater(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType compare_less_equal(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType compare_greater_equal(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    // Logical operators
    virtual BaseValue::PointerType logical_and(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType logical_or(const BaseValue* other) const { throw std::logic_error("method is not implemented for this type"); };
    virtual BaseValue::PointerType logical_not() const { throw std::logic_error("method is not implemented for this type"); };
    // Ternanry operators
    virtual BaseValue::PointerType where(const BaseValue* condition, const BaseValue* other) const = 0;
    // Qualifiers
    virtual bool any_of() const = 0;
    virtual bool all_of() const = 0;
    virtual bool none_of() const = 0;
    virtual bool is_unity() const = 0;
  };

} // namespace snt::val

#endif // VAL_BASE_VALUE_H
