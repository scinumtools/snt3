#ifndef VAL_BASE_VALUE_H
#define VAL_BASE_VALUE_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <snt/core/datatypes.h>
#include <snt/core/settings.h>
#include <snt/core/string_format.h>
#include <snt/val/array.h>
#include <snt/val/exceptions.h>
#include <sstream>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace snt::val {

    // Forward declaration
    template <typename T> class ArrayValue;

    // Define base value class
    class BaseValue {
      protected:
        Array::ShapeType shape;
        core::DataType dtype;

      public:
        using PointerType = std::unique_ptr<BaseValue>;
        BaseValue(core::DataType dt, Array::ShapeType sh) : dtype(dt), shape(sh) {};
        virtual ~BaseValue() = default;
        virtual void print() = 0;
        virtual std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const = 0;
        Array::ShapeType get_shape() const { return shape; };
        core::DataType get_dtype() const { return dtype; };
        virtual size_t get_size() const = 0;
        virtual BaseValue::PointerType clone() const = 0;
        virtual BaseValue::PointerType cast_as(core::DataType dt) const = 0;
        virtual BaseValue::PointerType slice(const Array::RangeType& slice) = 0;
        // Arithmetic operators
        virtual BaseValue::PointerType math_add(const BaseValue* other) const {
            throw val::MissingException("Method `math_add` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual void math_add_equal(const BaseValue* other) {
            throw val::MissingException("Method `math_add_equal` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_sub(const BaseValue* other) const {
            throw val::MissingException("Method `math_sub` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual void math_sub_equal(const BaseValue* other) {
            throw val::MissingException("Method `math_sub_equal` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_inv() const {
            throw val::MissingException("Method `math_inv` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_mul(const BaseValue* other) const {
            throw val::MissingException("Method `math_mul` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual void math_mul_equal(const BaseValue* other) {
            throw val::MissingException("Method `math_mul_equal` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_div(const BaseValue* other) const {
            throw val::MissingException("Method `math_div` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual void math_div_equal(const BaseValue* other) {
            throw val::MissingException("Method `math_div_equal` is not implemented for this type", __FILE__, __LINE__);
        };
        // Group operators
        virtual BaseValue::PointerType math_sin() const {
            throw val::MissingException("Method `math_sin` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_cos() const {
            throw val::MissingException("Method `math_cos` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_tan() const {
            throw val::MissingException("Method `math_tan` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_log() const {
            throw val::MissingException("Method `math_log` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_log10() const {
            throw val::MissingException("Method `math_log10` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_sqrt() const {
            throw val::MissingException("Method `math_sqrt` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_cbrt() const {
            throw val::MissingException("Method `math_cbrt` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_exp() const {
            throw val::MissingException("Method `math_exp` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_floor() const {
            throw val::MissingException("Method `math_floor` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_ceil() const {
            throw val::MissingException("Method `math_ceil` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_round() const {
            throw val::MissingException("Method `math_round` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_abs() const {
            throw val::MissingException("Method `math_abs` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_neg() const {
            throw val::MissingException("Method `math_neg` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_pow(const BaseValue* other) const {
            throw val::MissingException("Method `math_pow` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_pow(const double exp) const {
            throw val::MissingException("Method `math_pow` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_max(const BaseValue* other) const {
            throw val::MissingException("Method `math_max` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_min(const BaseValue* other) const {
            throw val::MissingException("Method `math_min` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType math_inf() const {
            throw val::MissingException("Method `math_inf` is not implemented for this type", __FILE__, __LINE__);
        };
        // Comparison operators
        virtual BaseValue::PointerType compare_equal(const BaseValue* other) const {
            throw val::MissingException("Method `compare_equal` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType compare_not_equal(const BaseValue* other) const {
            throw val::MissingException(
                "Method `compare_not_equal` is not implemented for this type", __FILE__, __LINE__
            );
        };
        virtual BaseValue::PointerType compare_less(const BaseValue* other) const {
            throw val::MissingException("Method `compare_less` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType compare_greater(const BaseValue* other) const {
            throw val::MissingException(
                "Method `compare_greater` is not implemented for this type", __FILE__, __LINE__
            );
        };
        virtual BaseValue::PointerType compare_less_equal(const BaseValue* other) const {
            throw val::MissingException(
                "Method `compare_less_equal` is not implemented for this type", __FILE__, __LINE__
            );
        };
        virtual BaseValue::PointerType compare_greater_equal(const BaseValue* other) const {
            throw val::MissingException(
                "Method `compare_greater_equal` is not implemented for this type", __FILE__, __LINE__
            );
        };
        // Logical operators
        virtual BaseValue::PointerType logical_and(const BaseValue* other) const {
            throw val::MissingException("Method `logical_and` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType logical_or(const BaseValue* other) const {
            throw val::MissingException("Method `logical_or` is not implemented for this type", __FILE__, __LINE__);
        };
        virtual BaseValue::PointerType logical_not() const {
            throw val::MissingException("Method `logical_not` is not implemented for this type", __FILE__, __LINE__);
        };
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
