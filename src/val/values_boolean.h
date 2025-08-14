#ifndef VAL_VALUES_BOOLEAN_H
#define VAL_VALUES_BOOLEAN_H

#include <typeinfo>

#include "values.h"

namespace val {

  template <>
  class ArrayValue<bool> : public BaseArrayValue<bool> {
  public:
    ArrayValue(const bool& val) : BaseArrayValue(val) {};
    ArrayValue(const std::vector<bool>& arr, const Array::ShapeType& sh) : BaseArrayValue(arr, sh) {};
    ArrayValue(const std::vector<bool>& arr) : BaseArrayValue(arr, {static_cast<int>(arr.size())}) {};
    ArrayValue(const BaseValue* other) : BaseArrayValue<bool>(other) {};

  private:
    void value_to_string(std::ostringstream& oss, size_t& offset, int precision = 0) const override {
      if (this->value[offset])
        oss << snt::KEYWORD_TRUE;
      else
        oss << snt::KEYWORD_FALSE;
    }

  public:
    std::string to_string(int precision = 0) const override {
      size_t offset = 0;
      if (precision == 0) {
        return to_string_dim(offset);
      } else {
        throw std::runtime_error("Boolean value does not support precision parameter for to_string() method.");
      }
    };
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
