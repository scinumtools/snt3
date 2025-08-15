#ifndef VAL_VALUES_STRING_H
#define VAL_VALUES_STRING_H

#include <typeinfo>

#include "values.h"

namespace val {

  template <>
  class ArrayValue<std::string> : public BaseArrayValue<std::string> {
  public:
    ArrayValue(const std::string& val) : BaseArrayValue(val) {};
    ArrayValue(const Array::StringType& arr, const Array::ShapeType& sh) : BaseArrayValue(arr, sh) {};
    ArrayValue(const Array::StringType& arr) : BaseArrayValue(arr, {static_cast<int>(arr.size())}) {};
    ArrayValue(const BaseValue* other) : BaseArrayValue<std::string>(other) {};

  private:
    void value_to_string(std::ostringstream& oss, size_t& offset, int precision = 0) const override {
      oss << "'" << value[offset] << "'";
    }

  public:
    std::string to_string(int precision = 0) const override {
      size_t offset = 0;
      if (precision == 0) {
        return to_string_dim(offset);
      } else {
        throw std::runtime_error("String value does not support precision parameter for to_string() method.");
      }
    };
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

} // namespace val

#endif // VAL_VALUES_STRING_H
