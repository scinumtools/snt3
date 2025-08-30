#ifndef VAL_VALUES_BOOLEAN_H
#define VAL_VALUES_BOOLEAN_H

#include "values_array.h"

#include <typeinfo>

namespace val {

  template <typename T>
  class ArrayValue;

  template <>
  class ArrayValue<bool> : public BaseArrayValue<bool> {
  public:
    ArrayValue(const bool& val) : BaseArrayValue(val) {};
    ArrayValue(const std::vector<bool>& arr, const Array::ShapeType& sh) : BaseArrayValue(arr, sh) {};
    ArrayValue(const std::vector<bool>& arr) : BaseArrayValue(arr, {arr.size()}) {};
    ArrayValue(const BaseValue* other) : BaseArrayValue<bool>(other) {};

  public:
    BaseValue::PointerType clone() const override {
      return std::make_unique<ArrayValue<bool>>(this->value, this->shape);
    };
    BaseValue::PointerType slice(const Array::RangeType& slice) override {
      return this->slice_value(slice);
    };
    BaseValue::PointerType cast_as(DataType dt) const override;
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
    bool is_unity() const override {
      return all_of();
    };
  };

} // namespace val

#endif // VAL_VALUES_BOOLEAN_H
