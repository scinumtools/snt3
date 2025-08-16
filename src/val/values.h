#ifndef VAL_VALUES_H
#define VAL_VALUES_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <cstddef>
#include <limits>

#include "../settings.h"

namespace val {

  enum class DataType { // TODO: INTX and FLOATX should support arbitrary precision types
    Boolean,
    String,
    Character,
    Integer16,
    Integer32,
    Integer64,
    IntegerX,
    Integer16_U,
    Integer32_U,
    Integer64_U,
    Float32,
    Float64,
    Float128,
    FloatX
  };

  enum class EvalMode {
    Piecewise,
    All,
    Any
  };

  extern std::unordered_map<DataType, std::string> DataTypeNames;

  // Define common array types
  namespace Array {
    struct RangeStruct {
      size_t dmin;
      size_t dmax;
      bool operator==(const RangeStruct& other) const {
        return (dmin == other.dmin) && (dmax == other.dmax);
      };
    };
    typedef std::vector<std::string> StringType; // holds raw string values
    typedef std::vector<RangeStruct> RangeType;  // array dimension ranges
    typedef std::vector<size_t> ShapeType;       // array shape
    constexpr size_t max_range = std::numeric_limits<size_t>::max();
  } // namespace Array

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
    virtual std::string to_string(const int precision = 0) const = 0;
    Array::ShapeType get_shape() const { return shape; };
    DataType get_dtype() const { return dtype; };
    virtual size_t get_size() const = 0;
    virtual BaseValue::PointerType clone() const = 0;
    virtual BaseValue::PointerType cast_as(DataType dt) const = 0;
    virtual BaseValue::PointerType slice(const Array::RangeType& slice) = 0;
    virtual BaseValue::PointerType math_log() const = 0;
    virtual BaseValue::PointerType math_log10() const = 0;
    virtual BaseValue::PointerType math_sqrt() const = 0;
    virtual BaseValue::PointerType math_cbrt() const = 0;
    virtual BaseValue::PointerType math_exp() const = 0;
    virtual BaseValue::PointerType math_floor() const = 0;
    virtual BaseValue::PointerType math_ceil() const = 0;
    virtual BaseValue::PointerType math_abs() const = 0;
    virtual BaseValue::PointerType math_neg() const = 0;
    virtual BaseValue::PointerType math_add(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType math_add(const double num) const = 0;
    virtual BaseValue::PointerType math_sub(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType math_sub(const double num) const = 0;
    virtual BaseValue::PointerType math_mul(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType math_mul(const double num) const = 0;
    virtual BaseValue::PointerType math_div(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType math_div(const double num) const = 0;
    virtual void math_add_equal(const BaseValue* other) = 0;
    virtual void math_sub_equal(const BaseValue* other) = 0;
    virtual void math_mul_equal(const BaseValue* other) = 0;
    virtual void math_div_equal(const BaseValue* other) = 0;
    virtual BaseValue::PointerType math_pow(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType math_pow(const double exp) const = 0;
    virtual BaseValue::PointerType math_max(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType math_min(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType compare_equal(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType compare_not_equal(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType compare_less(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType compare_greater(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType compare_less_equal(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType compare_greater_equal(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType logical_and(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType logical_or(const BaseValue* other) const = 0;
    virtual BaseValue::PointerType logical_not() const = 0;
    virtual BaseValue::PointerType where(const BaseValue* condition, const BaseValue* other) const = 0;
    virtual bool any_of() const = 0;
    virtual bool all_of() const = 0;
    virtual bool none_of() const = 0;
    virtual bool is_unity() const = 0;
  };

} // namespace val

#include "values_array.h"
#include "values_boolean.h"
#include "values_number.h"
#include "values_string.h"

namespace val {

  // helper function that create a array value pointer from a C++ data type
  template <typename T>
  BaseValue::PointerType create_array_value(const std::vector<T>& arr, Array::ShapeType sh = {}) {
    if (sh.empty())
      sh.push_back(arr.size());
    if constexpr (std::is_same_v<T, bool>)
      return std::make_unique<ArrayValue<bool>>(ArrayValue<bool>(arr, sh));
    else if constexpr (std::is_same_v<T, int16_t>)
      return std::make_unique<ArrayValue<int16_t>>(ArrayValue<int16_t>(arr, sh));
    else if constexpr (std::is_same_v<T, uint16_t>)
      return std::make_unique<ArrayValue<uint16_t>>(ArrayValue<uint16_t>(arr, sh));
    else if constexpr (std::is_same_v<T, int32_t>)
      return std::make_unique<ArrayValue<int32_t>>(ArrayValue<int32_t>(arr, sh));
    else if constexpr (std::is_same_v<T, uint32_t>)
      return std::make_unique<ArrayValue<uint32_t>>(ArrayValue<uint32_t>(arr, sh));
    else if constexpr (std::is_same_v<T, int64_t>)
      return std::make_unique<ArrayValue<int64_t>>(ArrayValue<int64_t>(arr, sh));
    else if constexpr (std::is_same_v<T, uint64_t>)
      return std::make_unique<ArrayValue<uint64_t>>(ArrayValue<uint64_t>(arr, sh));
    else if constexpr (std::is_same_v<T, float>)
      return std::make_unique<ArrayValue<float>>(ArrayValue<float>(arr, sh));
    else if constexpr (std::is_same_v<T, double>)
      return std::make_unique<ArrayValue<double>>(ArrayValue<double>(arr, sh));
    else if constexpr (std::is_same_v<T, long double>)
      return std::make_unique<ArrayValue<long double>>(ArrayValue<long double>(arr, sh));
    else if constexpr (std::is_same_v<T, std::string>)
      return std::make_unique<ArrayValue<std::string>>(ArrayValue<std::string>(arr, sh));
    else
      static_assert(std::is_integral_v<T>, "Given data type is not associated with any array value");
  };

} // namespace val

#endif // VAL_VALUES_H
