#ifndef VAL_VALUES_STRING_H
#define VAL_VALUES_STRING_H

#include <snt/val/array.h>
#include <snt/val/values_array.h>

#include <typeinfo>

namespace snt::val {

  template <typename T>
  class ArrayValue;

  template <>
  class ArrayValue<std::string> : public BaseArrayValue<std::string> {
  public:
    ArrayValue(const std::string& val) : BaseArrayValue(val) {};
    ArrayValue(const val::Array::StringType& arr, const val::Array::ShapeType& sh) : BaseArrayValue(arr, sh) {};
    ArrayValue(const val::Array::StringType& arr) : BaseArrayValue(arr, {arr.size()}) {};
    ArrayValue(const BaseValue* other) : BaseArrayValue<std::string>(other) {};

  public:
    BaseValue::PointerType clone() const override {
      return std::make_unique<ArrayValue<std::string>>(this->value, this->shape);
    };
    BaseValue::PointerType slice(const Array::RangeType& slice) override {
      return this->slice_value(slice);
    };
    BaseValue::PointerType cast_as(DataType dt) const override;
    BaseValue::PointerType logical_and(const BaseValue* other) const override;
    BaseValue::PointerType logical_or(const BaseValue* other) const override;
    BaseValue::PointerType logical_not() const override;
    bool any_of() const override {
      return std::any_of(this->value.begin(), this->value.end(), [](const std::string& b) { return !b.empty(); });
    };
    bool all_of() const override {
      return std::all_of(this->value.begin(), this->value.end(), [](const std::string& b) { return !b.empty(); });
    };
    bool none_of() const override {
      return std::none_of(this->value.begin(), this->value.end(), [](const std::string& b) { return !b.empty(); });
    };
    bool is_unity() const override {
      return std::all_of(this->value.begin(), this->value.end(), [](const std::string& b) { return std::stod(b) == 1; });
    };
  };

} // namespace snt::val

#endif // VAL_VALUES_STRING_H
