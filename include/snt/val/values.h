#ifndef VAL_VALUES_H
#define VAL_VALUES_H

#include <snt/val/values_number.h>
#include <snt/val/values_string.h>

namespace snt::val {

    class ArrayValueBool : public ArrayValue<uint8_t> {
      public:
        ArrayValueBool(const bool& val) : ArrayValue<uint8_t>(val, core::DataType::Boolean) {};
        ArrayValueBool(const std::vector<bool>& arr, const Array::ShapeType& sh)
            : ArrayValue<uint8_t>(std::vector<uint8_t>(arr.begin(), arr.end()), sh, core::DataType::Boolean) {};
        ArrayValueBool(const std::vector<bool>& arr)
            : ArrayValue<uint8_t>(std::vector<uint8_t>(arr.begin(), arr.end()), core::DataType::Boolean) {};
        ArrayValueBool(const BaseValue* other) : ArrayValue<uint8_t>(other, core::DataType::Boolean) {};
    };

    class ArrayValueChar : public ArrayValue<int8_t> {
      public:
        ArrayValueChar(const char& val) : ArrayValue<int8_t>(val, core::DataType::Character) {};
        ArrayValueChar(const std::vector<char>& arr, const Array::ShapeType& sh)
            : ArrayValue<int8_t>(std::vector<int8_t>(arr.begin(), arr.end()), sh, core::DataType::Character) {};
        ArrayValueChar(const std::vector<char>& arr)
            : ArrayValue<int8_t>(std::vector<int8_t>(arr.begin(), arr.end()), core::DataType::Character) {};
        ArrayValueChar(const BaseValue* other) : ArrayValue<int8_t>(other, core::DataType::Character) {};
    };

    class ArrayValueInt16 : public ArrayValue<int64_t> {
      public:
        ArrayValueInt16(const int16_t& val) : ArrayValue<int64_t>(val, core::DataType::Integer16) {};
        ArrayValueInt16(const std::vector<int16_t>& arr, const Array::ShapeType& sh)
            : ArrayValue<int64_t>(std::vector<int64_t>(arr.begin(), arr.end()), sh, core::DataType::Integer16) {};
        ArrayValueInt16(const std::vector<int16_t>& arr)
            : ArrayValue<int64_t>(std::vector<int64_t>(arr.begin(), arr.end()), core::DataType::Integer16) {};
        ArrayValueInt16(const BaseValue* other) : ArrayValue<int64_t>(other, core::DataType::Integer16) {};
    };

    class ArrayValueInt32 : public ArrayValue<int64_t> {
      public:
        ArrayValueInt32(const int32_t& val) : ArrayValue<int64_t>(val, core::DataType::Integer32) {};
        ArrayValueInt32(const std::vector<int32_t>& arr, const Array::ShapeType& sh)
            : ArrayValue<int64_t>(std::vector<int64_t>(arr.begin(), arr.end()), sh, core::DataType::Integer32) {};
        ArrayValueInt32(const std::vector<int32_t>& arr)
            : ArrayValue<int64_t>(std::vector<int64_t>(arr.begin(), arr.end()), core::DataType::Integer32) {};
        ArrayValueInt32(const BaseValue* other) : ArrayValue<int64_t>(other, core::DataType::Integer32) {};
    };

    class ArrayValueInt64 : public ArrayValue<int64_t> {
      public:
        ArrayValueInt64(const int64_t& val) : ArrayValue<int64_t>(val, core::DataType::Integer64) {};
        ArrayValueInt64(const std::vector<int64_t>& arr, const Array::ShapeType& sh)
            : ArrayValue<int64_t>(arr, sh, core::DataType::Integer64) {};
        ArrayValueInt64(const std::vector<int64_t>& arr) : ArrayValue<int64_t>(arr, core::DataType::Integer64) {};
        ArrayValueInt64(const BaseValue* other) : ArrayValue<int64_t>(other, core::DataType::Integer64) {};
    };

    class ArrayValueUint16 : public ArrayValue<uint64_t> {
      public:
        ArrayValueUint16(const uint16_t& val) : ArrayValue<uint64_t>(val, core::DataType::Integer16_U) {};
        ArrayValueUint16(const std::vector<uint16_t>& arr, const Array::ShapeType& sh)
            : ArrayValue<uint64_t>(std::vector<uint64_t>(arr.begin(), arr.end()), sh, core::DataType::Integer16_U) {};
        ArrayValueUint16(const std::vector<uint16_t>& arr)
            : ArrayValue<uint64_t>(std::vector<uint64_t>(arr.begin(), arr.end()), core::DataType::Integer16_U) {};
        ArrayValueUint16(const BaseValue* other) : ArrayValue<uint64_t>(other, core::DataType::Integer16_U) {};
    };

    class ArrayValueUint32 : public ArrayValue<uint64_t> {
      public:
        ArrayValueUint32(const uint32_t& val) : ArrayValue<uint64_t>(val, core::DataType::Integer32_U) {};
        ArrayValueUint32(const std::vector<uint32_t>& arr, const Array::ShapeType& sh)
            : ArrayValue<uint64_t>(std::vector<uint64_t>(arr.begin(), arr.end()), sh, core::DataType::Integer32_U) {};
        ArrayValueUint32(const std::vector<uint32_t>& arr)
            : ArrayValue<uint64_t>(std::vector<uint64_t>(arr.begin(), arr.end()), core::DataType::Integer32_U) {};
        ArrayValueUint32(const BaseValue* other) : ArrayValue<uint64_t>(other, core::DataType::Integer32_U) {};
    };

    class ArrayValueUint64 : public ArrayValue<uint64_t> {
      public:
        ArrayValueUint64(const uint64_t& val) : ArrayValue<uint64_t>(val, core::DataType::Integer64_U) {};
        ArrayValueUint64(const std::vector<uint64_t>& arr, const Array::ShapeType& sh)
            : ArrayValue<uint64_t>(arr, sh, core::DataType::Integer64_U) {};
        ArrayValueUint64(const std::vector<uint64_t>& arr) : ArrayValue<uint64_t>(arr, core::DataType::Integer64_U) {};
        ArrayValueUint64(const BaseValue* other) : ArrayValue<uint64_t>(other, core::DataType::Integer64_U) {};
    };

    class ArrayValueFloat32 : public ArrayValue<double> {
      public:
        ArrayValueFloat32(const float& val) : ArrayValue<double>(val, core::DataType::Float32) {};
        ArrayValueFloat32(const std::vector<float>& arr, const Array::ShapeType& sh)
            : ArrayValue<double>(std::vector<double>(arr.begin(), arr.end()), sh, core::DataType::Float32) {};
        ArrayValueFloat32(const std::vector<float>& arr)
            : ArrayValue<double>(std::vector<double>(arr.begin(), arr.end()), core::DataType::Float32) {};
        ArrayValueFloat32(const BaseValue* other) : ArrayValue<double>(other, core::DataType::Float32) {};
        void get_data(std::vector<float>& output_value) const { output_value.assign(value.begin(), value.end()); };
    };

    class ArrayValueFloat64 : public ArrayValue<double> {
      public:
        ArrayValueFloat64(const double& val) : ArrayValue<double>(val, core::DataType::Float64) {};
        ArrayValueFloat64(const std::vector<double>& arr, const Array::ShapeType& sh)
            : ArrayValue<double>(arr, sh, core::DataType::Float64) {};
        ArrayValueFloat64(const std::vector<double>& arr) : ArrayValue<double>(arr, core::DataType::Float64) {};
        ArrayValueFloat64(const BaseValue* other) : ArrayValue<double>(other, core::DataType::Float64) {};
    };

    class ArrayValueFloat128 : public ArrayValue<long double> {
      public:
        ArrayValueFloat128(const long double& val) : ArrayValue<long double>(val, core::DataType::Float128) {};
        ArrayValueFloat128(const std::vector<long double>& arr, const Array::ShapeType& sh)
            : ArrayValue<long double>(arr, sh, core::DataType::Float128) {};
        ArrayValueFloat128(const std::vector<long double>& arr)
            : ArrayValue<long double>(arr, core::DataType::Float128) {};
        ArrayValueFloat128(const BaseValue* other) : ArrayValue<long double>(other, core::DataType::Float128) {};
    };

    class ArrayValueStr : public ArrayValue<std::string> {
      public:
        ArrayValueStr(const std::string& val) : ArrayValue<std::string>(val, core::DataType::String) {};
        ArrayValueStr(const val::Array::StringType& arr, const val::Array::ShapeType& sh)
            : ArrayValue<std::string>(arr, sh, core::DataType::String) {};
        ArrayValueStr(const val::Array::StringType& arr)
            : ArrayValue<std::string>(arr, {arr.size()}, core::DataType::String) {};
        ArrayValueStr(const BaseValue* other) : ArrayValue<std::string>(other, core::DataType::String) {};
    };

} // namespace snt::val

#endif // VAL_VALUES_H
