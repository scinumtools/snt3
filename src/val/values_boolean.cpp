#include "values.h"

namespace val {

  BaseValue::PointerType ArrayValue<bool>::cast_as(DataType dt) const {
    switch (dt) {
    case DataType::Boolean: {
      return std::make_unique<ArrayValue<bool>>(this->value, this->shape);
    }
    case DataType::Character: {
      std::vector<char> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<char>(this->value[i]);
      return std::make_unique<ArrayValue<char>>(arr, this->shape);
    }
    case DataType::Integer16: {
      std::vector<int16_t> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<int16_t>(this->value[i]);
      return std::make_unique<ArrayValue<int16_t>>(arr, this->shape);
    }
    case DataType::Integer32: {
      std::vector<int32_t> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<int32_t>(this->value[i]);
      return std::make_unique<ArrayValue<int32_t>>(arr, this->shape);
    }
    case DataType::Integer64: {
      std::vector<int64_t> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<int64_t>(this->value[i]);
      return std::make_unique<ArrayValue<int64_t>>(arr, this->shape);
    }
    case DataType::Integer16_U: {
      std::vector<uint16_t> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<uint16_t>(this->value[i]);
      return std::make_unique<ArrayValue<uint16_t>>(arr, this->shape);
    }
    case DataType::Integer32_U: {
      std::vector<uint32_t> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<uint32_t>(this->value[i]);
      return std::make_unique<ArrayValue<uint32_t>>(arr, this->shape);
    }
    case DataType::Integer64_U: {
      std::vector<uint64_t> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<uint64_t>(this->value[i]);
      return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape);
    }
    case DataType::Float32: {
      std::vector<float> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<float>(this->value[i]);
      return std::make_unique<ArrayValue<float>>(arr, this->shape);
    }
    case DataType::Float64: {
      std::vector<double> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<double>(this->value[i]);
      return std::make_unique<ArrayValue<double>>(arr, this->shape);
    }
    case DataType::Float128: {
      std::vector<long double> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++)
        arr[i] = static_cast<long double>(this->value[i]);
      return std::make_unique<ArrayValue<long double>>(arr, this->shape);
    }
    case DataType::String: {
      std::vector<std::string> arr(this->value.size());
      for (size_t i = 0; i < this->value.size(); i++) {
        if (this->value[i])
          arr[i] = std::string(snt::KEYWORD_TRUE);
        else
          arr[i] = std::string(snt::KEYWORD_FALSE);
      }
      return std::make_unique<ArrayValue<std::string>>(arr, this->shape);
    }
    default:
      throw std::runtime_error("Not implemented");
      return nullptr;
    }
  };

  BaseValue::PointerType ArrayValue<bool>::logical_and(const BaseValue* other) const {
    return operate_binary<bool>(other, [](bool a, bool b) { return a && b; });
  };

  BaseValue::PointerType ArrayValue<bool>::logical_or(const BaseValue* other) const {
    return operate_binary<bool>(other, [](bool a, bool b) { return a || b; });
  };

  BaseValue::PointerType ArrayValue<bool>::logical_not() const {
    return operate_unary<bool>([](bool a) { return !a; });
  };

} // namespace val
