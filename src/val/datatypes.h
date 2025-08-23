#ifndef VAL_DATATYPES_H
#define VAL_DATATYPES_H

#include <string>
#include <unordered_map>
#include <type_traits>
#include <cstdint>

namespace val {
  
  template <typename E>
  constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
  }
  
  template <typename E>
  constexpr E operator|(E lhs, E rhs) {
    using T = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) | static_cast<T>(rhs));
  }
  
  template <typename E>
  constexpr E& operator|=(E& lhs, E rhs) {
    lhs = lhs | rhs;
    return lhs;
  }
  
  enum class DataType: uint16_t {
    // Define individual data types
    None           = 0,
    Boolean        = 1 << 0,
    String         = 1 << 1,
    Character      = 1 << 2,
    Integer16      = 1 << 3,
    Integer32      = 1 << 4,
    Integer64      = 1 << 5,
    IntegerX       = 1 << 6,   // TODO: need to implement
    Integer16_U    = 1 << 7,
    Integer32_U    = 1 << 8,
    Integer64_U    = 1 << 9,
    Float32        = 1 << 10,
    Float64        = 1 << 11,
    Float128       = 1 << 12,  // TODO: this is not percise, could be less precision
    FloatX         = 1 << 13,  // TODO: need to implement

    // Define data type groups
    Integer = Integer16 | Integer32 | Integer64 | IntegerX | 
    Integer16_U | Integer32_U | Integer64_U,

    Integer_S = Integer16 | Integer32 | Integer64 | IntegerX,
    Integer_U = Integer16_U | Integer32_U | Integer64_U,
      
    Float = Float32 | Float64 | Float128 | FloatX

  };

  inline DataType operator|(DataType lhs, DataType rhs) {
    return static_cast<DataType>(static_cast<std::underlying_type_t<DataType>>(lhs) |
				 static_cast<std::underlying_type_t<DataType>>(rhs));
  }
  
  inline DataType operator&(DataType lhs, DataType rhs) {
    return static_cast<DataType>(static_cast<std::underlying_type_t<DataType>>(lhs) &
				 static_cast<std::underlying_type_t<DataType>>(rhs));
  }
  
  inline DataType& operator|=(DataType& lhs, DataType rhs) {
    lhs = lhs | rhs;
    return lhs;
  }
  
  inline DataType operator~(DataType f) {
    return static_cast<DataType>(~static_cast<std::underlying_type_t<DataType>>(f));
  }

  inline bool any(DataType a) {
    return static_cast<uint16_t>(a) != 0;
  }
  
  extern std::unordered_map<DataType, std::string> DataTypeNames;

}

#endif // VAL_DATATYPES_H
