#ifndef VAL_VALUES_BOOLEAN_H
#define VAL_VALUES_BOOLEAN_H

#include <snt/val/values_array.h>
#include <typeinfo>

namespace snt::val {

    template <typename T> class ArrayValue;

    template <> class ArrayValue<bool> : public BaseArrayValue<bool> {
      public:
        ArrayValue(const bool& val, const core::DataType dtype) : BaseArrayValue(val, dtype) {};
        ArrayValue(const std::vector<bool>& arr, const Array::ShapeType& sh, const core::DataType dtype)
            : BaseArrayValue(arr, sh, dtype) {};
        ArrayValue(const std::vector<bool>& arr, const core::DataType dtype)
            : BaseArrayValue(arr, {arr.size()}, dtype) {};
        ArrayValue(const BaseValue* other, const core::DataType dtype) : BaseArrayValue<bool>(other, dtype) {};

      public:
        BaseValue::PointerType clone() const override {
            return std::make_unique<ArrayValue<bool>>(this->value, this->shape, this->dtype);
        };
        BaseValue::PointerType slice(const Array::RangeType& slice) override { return this->slice_value(slice); };
        BaseValue::PointerType cast_as(core::DataType dt) const override;
        BaseValue::PointerType logical_and(const BaseValue* other) const override;
        BaseValue::PointerType logical_or(const BaseValue* other) const override;
        BaseValue::PointerType logical_not() const override;
        bool any_of() const override {
            return std::any_of(this->value.begin(), this->value.end(), [](bool b) { return b; });
        };
        bool all_of() const override {
            return std::all_of(this->value.begin(), this->value.end(), [](bool b) { return b; });
        };
        bool none_of() const override {
            return std::none_of(this->value.begin(), this->value.end(), [](bool b) { return b; });
        };
        bool is_unity() const override { return all_of(); };
    };

} // namespace snt::val

#endif // VAL_VALUES_BOOLEAN_H
