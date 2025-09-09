#include "datatypes.h"

namespace snt::val {

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
      {DataType::Real32, "real32"},
      {DataType::Real64, "real64"},
      {DataType::Real128, "real128"},
      {DataType::RealX, "realx"},

      {DataType::Integer, "int"},
      {DataType::Integer_S, "sint"},
      {DataType::Integer_U, "uint"},
      {DataType::Real, "real"},
  };

}
