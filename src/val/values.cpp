#include "values.h"

#include <map>

namespace val {

  std::unordered_map<DataType, std::string> DataTypeNames = {
      {DataType::Boolean, "bool"},
      {DataType::String, "str"},
      {DataType::Character, "char"},
      {DataType::Integer16, "int16"},
      {DataType::Integer32, "int32"},
      {DataType::Integer64, "int64"},
      {DataType::IntegerX, "intx"},
      {DataType::Integer16_U, "uint16"},
      {DataType::Integer32_U, "uint32"},
      {DataType::Integer64_U, "uint64"},
      {DataType::Float32, "float32"},
      {DataType::Float64, "float64"},
      {DataType::Float128, "float128"},
      {DataType::FloatX, "floatx"}};

}
