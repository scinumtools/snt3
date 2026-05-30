#ifndef VAL_VALUES_NUMBER_H
#define VAL_VALUES_NUMBER_H

#include <limits>
#include <snt/val/values_array.h>
#include <typeinfo>

namespace snt::val {

    template <typename T> class ArrayValue;

    template <typename T> class ArrayValue : public BaseArrayValue<T> {
      public:
        ArrayValue(const T& val, const core::DataType dtype) : BaseArrayValue<T>(val, dtype) {};
        ArrayValue(const std::vector<T>& arr, const Array::ShapeType& sh, const core::DataType dtype)
            : BaseArrayValue<T>(arr, sh, dtype) {};
        ArrayValue(const std::vector<T>& arr, const core::DataType dtype)
            : BaseArrayValue<T>(arr, {arr.size()}, dtype) {};
        ArrayValue(const BaseValue* other, const core::DataType dtype) : BaseArrayValue<T>(other, dtype) {};

      public:
        BaseValue::PointerType clone() const override {
            return std::make_unique<ArrayValue<T>>(this->value, this->shape, this->dtype);
        };
        BaseValue::PointerType cast_as(core::DataType dt) const override {
            switch (dt) {
            case core::DataType::Boolean: {
                std::vector<uint8_t> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<uint8_t>(this->value[i]);
                return std::make_unique<ArrayValue<uint8_t>>(arr, this->shape, dt);
            }
            case core::DataType::Character: {
                std::vector<int8_t> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<int8_t>(this->value[i]);
                return std::make_unique<ArrayValue<int8_t>>(arr, this->shape, dt);
            }
            case core::DataType::Integer16: {
                std::vector<int64_t> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<int64_t>(this->value[i]);
                return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
            }
            case core::DataType::Integer32: {
                std::vector<int64_t> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<int64_t>(this->value[i]);
                return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
            }
            case core::DataType::Integer64: {
                std::vector<int64_t> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<int64_t>(this->value[i]);
                return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
            }
            case core::DataType::Integer16_U: {
                std::vector<uint64_t> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<uint64_t>(this->value[i]);
                return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
            }
            case core::DataType::Integer32_U: {
                std::vector<uint64_t> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<uint64_t>(this->value[i]);
                return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
            }
            case core::DataType::Integer64_U: {
                std::vector<uint64_t> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<uint64_t>(this->value[i]);
                return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
            }
            case core::DataType::Float32: {
                std::vector<double> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<double>(this->value[i]);
                return std::make_unique<ArrayValue<double>>(arr, this->shape, dt);
            }
            case core::DataType::Float64: {
                std::vector<double> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<double>(this->value[i]);
                return std::make_unique<ArrayValue<double>>(arr, this->shape, dt);
            }
            case core::DataType::Float128: {
                std::vector<long double> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++)
                    arr[i] = static_cast<long double>(this->value[i]);
                return std::make_unique<ArrayValue<long double>>(arr, this->shape, dt);
            }
            case core::DataType::String: {
                std::vector<std::string> arr(this->value.size());
                for (size_t i = 0; i < this->value.size(); i++) {
                    if (this->dtype == core::DataType::Boolean) {
                        if (this->value[i])
                            arr[i] = std::string(core::KEYWORD_TRUE);
                        else
                            arr[i] = std::string(core::KEYWORD_FALSE);
                    } else {
                        arr[i] = std::to_string(this->value[i]);
                    }
                }
                return std::make_unique<ArrayValue<std::string>>(arr, this->shape, dt);
            }
            default:
                throw std::runtime_error("Not implemented");
                return nullptr;
            }
        };
        BaseValue::PointerType slice(const Array::RangeType& slice) override { return this->slice_value(slice); };
        BaseValue::PointerType math_sin() const override {
            return this->template operate_unary<T>([](T a) { return std::sin(a); });
        };
        BaseValue::PointerType math_cos() const override {
            return this->template operate_unary<T>([](T a) { return std::cos(a); });
        };
        BaseValue::PointerType math_tan() const override {
            return this->template operate_unary<T>([](T a) { return std::tan(a); });
        };
        BaseValue::PointerType math_log() const override {
            return this->template operate_unary<T>([](T a) { return std::log(a); });
        };
        BaseValue::PointerType math_log10() const override {
            return this->template operate_unary<T>([](T a) { return std::log10(a); });
        };
        BaseValue::PointerType math_sqrt() const override {
            return this->template operate_unary<T>([](T a) { return std::sqrt(a); });
        };
        BaseValue::PointerType math_cbrt() const override {
            return this->template operate_unary<T>([](T a) { return std::cbrt(a); });
        };
        BaseValue::PointerType math_exp() const override {
            return this->template operate_unary<T>([](T a) { return std::exp(a); });
        };
        BaseValue::PointerType math_floor() const override {
            return this->template operate_unary<T>([](T a) { return std::floor(a); });
        };
        BaseValue::PointerType math_ceil() const override {
            return this->template operate_unary<T>([](T a) { return std::ceil(a); });
        };
        BaseValue::PointerType math_round() const override {
            return this->template operate_unary<T>([](T a) { return std::round(a); });
        };
        BaseValue::PointerType math_abs() const override {
            if constexpr (std::is_unsigned_v<T>) // is meaningless because an unsigned value is never negative.
                return this->clone();
            else
                return this->template operate_unary<T>([](T a) { return std::abs(a); });
        };
        // negation
        BaseValue::PointerType math_neg() const override {
            return this->template operate_unary<T>([](T a) { return -a; });
        };
        // addition
        BaseValue::PointerType math_add(const BaseValue* other) const override {
            if (any(this->dtype & core::DataType::Integer) && any(other->get_dtype() & core::DataType::Float))
                return other->math_add(this);
            else
                return this->template operate_binary<T>(other, [](T a, T b) { return a + b; });
        };
        void math_add_equal(const BaseValue* other) override {
            this->template operate_binary_equal<T>(other, [](T a, T b) { return a + b; });
        };
        // subtraction
        BaseValue::PointerType math_sub(const BaseValue* other) const override {
            if (any(this->dtype & core::DataType::Integer) && any(other->get_dtype() & core::DataType::Float))
                return other->math_neg()->math_add(this);
            else
                return this->template operate_binary<T>(other, [](T a, T b) { return a - b; });
        };
        void math_sub_equal(const BaseValue* other) override {
            this->template operate_binary_equal<T>(other, [](T a, T b) { return a - b; });
        };
        // inversion
        BaseValue::PointerType math_inv() const override {
            return this->template operate_unary<T>([](T a) { return 1. / a; });
        };
        // multiplication
        BaseValue::PointerType math_mul(const BaseValue* other) const override {
            if (any(this->dtype & core::DataType::Integer) && any(other->get_dtype() & core::DataType::Float))
                return other->math_mul(this);
            else
                return this->template operate_binary<T>(other, [](T a, T b) { return a * b; });
        };
        void math_mul_equal(const BaseValue* other) override {
            this->template operate_binary_equal<T>(other, [](T a, T b) { return a * b; });
        };
        // division
        BaseValue::PointerType math_div(const BaseValue* other) const override {
            if (any(this->dtype & core::DataType::Integer) && any(other->get_dtype() & core::DataType::Float))
                return other->math_inv()->math_mul(this);
            else
                return this->template operate_binary<T>(other, [](T a, T b) { return a / b; });
        };
        void math_div_equal(const BaseValue* other) override {
            this->template operate_binary_equal<T>(other, [](T a, T b) { return a / b; });
        };
        // power
        BaseValue::PointerType math_pow(const BaseValue* other) const override {
            return this->template operate_binary<T>(other, [](T a, T b) { return std::pow(a, b); });
        };
        BaseValue::PointerType math_pow(const double exp) const override {
            return this->template operate_unary<T>([&exp](T a) { return std::pow(a, exp); });
        };
        // maximum / minimum
        BaseValue::PointerType math_max(const BaseValue* other) const override {
            return this->template operate_binary<T>(other, [](T a, T b) { return std::max(a, b); });
        };
        BaseValue::PointerType math_min(const BaseValue* other) const override {
            return this->template operate_binary<T>(other, [](T a, T b) { return std::min(a, b); });
        };
        // extremems
        BaseValue::PointerType math_inf() const override {
            return this->template operate_unary<T>([](T a) { return std::numeric_limits<T>::infinity(); });
        };
        // logical
        BaseValue::PointerType logical_and(const BaseValue* other) const override {
            return this->template operate_binary<bool>(other, [](T a, T b) { return a && b; });
        };
        BaseValue::PointerType logical_or(const BaseValue* other) const override {
            return this->template operate_binary<bool>(other, [](T a, T b) { return a || b; });
        };
        BaseValue::PointerType logical_not() const override {
            return this->template operate_unary<bool>([](T a) { return !a; });
        };
        // quantifies
        bool any_of() const override {
            return std::any_of(this->value.begin(), this->value.end(), [](T b) { return b; });
        };
        bool all_of() const override {
            return std::all_of(this->value.begin(), this->value.end(), [](T b) { return b; });
        };
        bool none_of() const override {
            return std::none_of(this->value.begin(), this->value.end(), [](T b) { return b; });
        };
        bool is_unity() const override {
            return std::all_of(this->value.begin(), this->value.end(), [](T b) { return b == 1; });
        };
    };

} // namespace snt::val

#endif // VAL_VALUES_NUMBER_H
