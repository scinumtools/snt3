#ifndef VAL_DATATYPES_H
#define VAL_DATATYPES_H

#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace snt::val {

  /**
   * @brief Converts a strongly-typed enum value to its underlying integral type.
   *
   * @tparam E Enumeration type.
   * @param e Enum value to convert.
   * @return The underlying integral value corresponding to @p e.
   */
  template <typename E>
  constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
  }

  /**
   * @brief Performs a bitwise OR operation between two enum class values.
   *
   * @tparam E Enumeration type.
   * @param lhs Left-hand side operand.
   * @param rhs Right-hand side operand.
   * @return A new enum value representing the union of both flags.
   */
  template <typename E>
  constexpr E operator|(E lhs, E rhs) {
    using T = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) | static_cast<T>(rhs));
  }

  /**
   * @brief Performs an in-place bitwise OR assignment on enum class values.
   *
   * @tparam E Enumeration type.
   * @param lhs Left-hand side operand to modify.
   * @param rhs Right-hand side operand.
   * @return Reference to the modified @p lhs.
   */
  template <typename E>
  constexpr E& operator|=(E& lhs, E rhs) {
    lhs = lhs | rhs;
    return lhs;
  }

  /**
   * @enum DataType
   * @brief List of value data types
   */
  enum class DataType : uint16_t {
    // Define individual data types
    None = 0,               
    Boolean = 1 << 0,       
    String = 1 << 1,        
    Character = 1 << 2,
    Integer16 = 1 << 3,
    Integer32 = 1 << 4,
    Integer64 = 1 << 5,
    IntegerX = 1 << 6, // TODO: need to be implemented
    Integer16_U = 1 << 7,
    Integer32_U = 1 << 8,
    Integer64_U = 1 << 9,
    Float32 = 1 << 10,
    Float64 = 1 << 11,
    Float128 = 1 << 12, // TODO: this is not percise, could be less precision
    FloatX = 1 << 13,   // TODO: need to be implemented

    // Define data type groups
    Integer = Integer16 | Integer32 | Integer64 | IntegerX |
              Integer16_U | Integer32_U | Integer64_U,

    Integer_S = Integer16 | Integer32 | Integer64 | IntegerX,
    Integer_U = Integer16_U | Integer32_U | Integer64_U,

    Float = Float32 | Float64 | Float128 | FloatX

  };

  /// @brief Maps each DataType enumeration value to its human-readable name.
  ///
  /// This lookup table provides string representations of all defined DataType
  /// values. It is primarily used for serialization, logging, and diagnostic
  /// output where textual names of data types are required.
  extern std::unordered_map<DataType, std::string> DataTypeNames;

  /**
   * @brief Performs a bitwise OR operation on two DataType flags.
   *
   * @param lhs Left-hand side flag.
   * @param rhs Right-hand side flag.
   * @return A new DataType value representing the combined flags.
   */
  inline DataType operator|(DataType lhs, DataType rhs) {
    return static_cast<DataType>(static_cast<std::underlying_type_t<DataType>>(lhs) |
                                 static_cast<std::underlying_type_t<DataType>>(rhs));
  }

  /**
   * @brief Performs a bitwise AND operation on two DataType flags.
   *
   * @param lhs Left-hand side flag.
   * @param rhs Right-hand side flag.
   * @return A new DataType value representing the intersected flags.
   */   
  inline DataType operator&(DataType lhs, DataType rhs) {
    return static_cast<DataType>(static_cast<std::underlying_type_t<DataType>>(lhs) &
                                 static_cast<std::underlying_type_t<DataType>>(rhs));
  }
  
  /**
   * @brief Performs an in-place bitwise OR assignment on DataType flags.
   *
   * @param lhs Left-hand side flag reference to modify.
   * @param rhs Right-hand side flag.
   * @return Reference to the modified @p lhs.
   */
  inline DataType& operator|=(DataType& lhs, DataType rhs) {
    lhs = lhs | rhs;
    return lhs;
  }

  /**
   * @brief Performs a bitwise NOT operation on a DataType flag.
   *
   * @param f Input flag to invert.
   * @return A new DataType value with inverted bits.
   */
  inline DataType operator~(DataType f) {
    return static_cast<DataType>(~static_cast<std::underlying_type_t<DataType>>(f));
  }

  /**
   * @brief Test if there is some flag in a flag set
   *
   * @param a Input flag
   * @return Returns true if some flag is a set
   */
  inline bool any(DataType a) {
    return static_cast<uint16_t>(a) != 0;
  }

} // namespace snt::val

#endif // VAL_DATATYPES_H
