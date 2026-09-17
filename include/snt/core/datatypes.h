#ifndef SNT_CORE_DATATYPES_H
#define SNT_CORE_DATATYPES_H

#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace snt::core {

    /**
     * Converts a strongly-typed enum value to its underlying integral type.
     *
     * @tparam E Enumeration type.
     * @param e Enum value to convert.
     * @return The underlying integral value corresponding to @p e.
     */
    template <typename E> constexpr auto to_underlying(E e) noexcept {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    /**
     * Performs a bitwise OR operation between two enum class values.
     *
     * @tparam E Enumeration type.
     * @param lhs Left-hand side operand.
     * @param rhs Right-hand side operand.
     * @return A new enum value representing the union of both flags.
     */
    template <typename E> constexpr E operator|(E lhs, E rhs) {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    /**
     * Performs an in-place bitwise OR assignment on enum class values.
     *
     * @tparam E Enumeration type.
     * @param lhs Left-hand side operand to modify.
     * @param rhs Right-hand side operand.
     * @return Reference to the modified @p lhs.
     */
    template <typename E> constexpr E& operator|=(E& lhs, E rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    /**
     * List of value data types
     */
    /** Bit flags describing the scalar and grouped data types supported by SNT. */
    enum class DataType : uint16_t {
        // Define individual data types
        None = 0,             ///< No type.
        Boolean = 1 << 0,     ///< Boolean value.
        String = 1 << 1,      ///< UTF-8 string value.
        Character = 1 << 2,   ///< Character value.
        Integer16 = 1 << 3,   ///< Signed 16-bit integer.
        Integer32 = 1 << 4,   ///< Signed 32-bit integer.
        Integer64 = 1 << 5,   ///< Signed 64-bit integer.
        IntegerX = 1 << 6,    ///< Extended signed integer (reserved).
        Integer16_U = 1 << 7, ///< Unsigned 16-bit integer.
        Integer32_U = 1 << 8, ///< Unsigned 32-bit integer.
        Integer64_U = 1 << 9, ///< Unsigned 64-bit integer.
        Float32 = 1 << 10,    ///< 32-bit floating-point value.
        Float64 = 1 << 11,    ///< 64-bit floating-point value.
        Float128 = 1 << 12,   ///< Extended precision floating-point value.
        FloatX = 1 << 13,     ///< Extended floating-point type (reserved).
        Integer8 = 1 << 14,   ///< Signed 8-bit integer.
        Integer8_U = 1 << 15, ///< Unsigned 8-bit integer.

        // Define data type groups
        Integer = Integer8 | Integer16 | Integer32 | Integer64 | IntegerX | Integer8_U | Integer16_U | Integer32_U |
            Integer64_U,
        ///< Group containing all signed and unsigned integer types.

        Integer_S = Integer8 | Integer16 | Integer32 | Integer64 | IntegerX, ///< Group containing signed integers.
        Integer_U = Integer8_U | Integer16_U | Integer32_U | Integer64_U,    ///< Group containing unsigned integers.

        Float = Float32 | Float64 | Float128 | FloatX ///< Group containing floating-point types.

    };

    ///  Maps each DataType enumeration value to its human-readable name.
    ///
    /// This lookup table provides string representations of all defined DataType
    /// values. It is primarily used for serialization, logging, and diagnostic
    /// output where textual names of data types are required.
    extern std::unordered_map<DataType, std::string> DataTypeNames;

    /**
     * Performs a bitwise OR operation on two DataType flags.
     *
     * @param lhs Left-hand side flag.
     * @param rhs Right-hand side flag.
     * @return A new DataType value representing the combined flags.
     */
    inline DataType operator|(DataType lhs, DataType rhs) {
        return static_cast<DataType>(
            static_cast<std::underlying_type_t<DataType>>(lhs) | static_cast<std::underlying_type_t<DataType>>(rhs)
        );
    }

    /**
     * Performs a bitwise AND operation on two DataType flags.
     *
     * @param lhs Left-hand side flag.
     * @param rhs Right-hand side flag.
     * @return A new DataType value representing the intersected flags.
     */
    inline DataType operator&(DataType lhs, DataType rhs) {
        return static_cast<DataType>(
            static_cast<std::underlying_type_t<DataType>>(lhs) & static_cast<std::underlying_type_t<DataType>>(rhs)
        );
    }

    /**
     * Performs an in-place bitwise OR assignment on DataType flags.
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
     * Performs a bitwise NOT operation on a DataType flag.
     *
     * @param f Input flag to invert.
     * @return A new DataType value with inverted bits.
     */
    inline DataType operator~(DataType f) {
        return static_cast<DataType>(~static_cast<std::underlying_type_t<DataType>>(f));
    }

    /**
     * Test if there is some flag in a flag set
     *
     * @param a Input flag
     * @return Returns true if some flag is a set
     */
    inline bool any(DataType a) {
        return static_cast<uint16_t>(a) != 0;
    }

} // namespace snt::core

#endif // SNT_CORE_DATATYPES_H
