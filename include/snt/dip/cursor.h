#ifndef DIP_CURSOR_H
#define DIP_CURSOR_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/path.h>
#include <snt/val/value_base.h>
#include <snt/val/values_number.h>
#include <snt/val/values_string.h>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace snt::dip {

    namespace detail {

        template <typename T>
        inline constexpr bool is_cursor_element_v = std::is_same_v<T, bool> || std::is_integral_v<T> ||
                                                    std::is_floating_point_v<T> || std::is_same_v<T, std::string>;

        template <typename Target, typename Source>
        inline constexpr bool is_cursor_compatible_v =
            (std::is_same_v<Source, uint8_t> && std::is_same_v<Target, bool>) ||
            (std::is_same_v<Source, int64_t> && std::is_integral_v<Target> && std::is_signed_v<Target> &&
             !std::is_same_v<Target, bool>) ||
            (std::is_same_v<Source, uint64_t> && std::is_integral_v<Target> && std::is_unsigned_v<Target> &&
             !std::is_same_v<Target, bool>) ||
            (std::is_floating_point_v<Source> && std::is_floating_point_v<Target>) ||
            (std::is_same_v<Source, std::string> && std::is_same_v<Target, std::string>);

        [[noreturn]] inline void throw_cursor_type_mismatch(const std::string& path) {
            throw dip::SyntaxException(
                "Type mismatch",
                "The value of node `" + path + "` cannot be converted to the requested type.",
                "Request a type in the same DIPL value category.",
                __FILE__,
                __LINE__
            );
        }

        template <typename Target, typename Source> Target cursor_cast_element(const Source& value) {
            if constexpr (std::is_same_v<Target, bool>)
                return value != 0;
            else if constexpr (std::is_same_v<Target, std::string>)
                return value;
            else
                return static_cast<Target>(value);
        }

        template <typename T, typename Source>
        T cursor_cast_value(const val::ArrayValue<Source>& source, const std::string& path) {
            using ValueType = element_type_t<T>;
            static_assert(
                is_cursor_element_v<ValueType>,
                "Cursor::as<T>() supports scalar values, std::vector, and std::array of DIPL value types."
            );
            if constexpr (!is_cursor_compatible_v<ValueType, Source>) {
                throw_cursor_type_mismatch(path);
            } else if constexpr (is_std_vector_v<T>) {
                T result;
                result.reserve(source.get_size());
                for (const auto& value : source.get_values())
                    result.push_back(cursor_cast_element<ValueType>(value));
                return result;
            } else if constexpr (is_std_array_v<T>) {
                if (source.get_size() != std::tuple_size_v<T>)
                    throw dip::EnvironmentException(
                        "Array size mismatch",
                        "The value of node `" + path + "` contains " + std::to_string(source.get_size()) +
                            " elements, but the requested array contains " + std::to_string(std::tuple_size_v<T>) +
                            " elements.",
                        "Request an array with the same number of elements as the node value.",
                        __FILE__,
                        __LINE__
                    );
                T result;
                const auto values = source.get_values();
                std::transform(values.begin(), values.end(), result.begin(), [](const auto& value) {
                    return cursor_cast_element<ValueType>(value);
                });
                return result;
            } else {
                if (source.get_size() != 1)
                    throw dip::SyntaxException(
                        "Expected scalar value",
                        "The value of node `" + path + "` contains " + std::to_string(source.get_size()) +
                            " elements, but a scalar value was requested.",
                        "Request a scalar node value or use a vector or array type for multiple elements.",
                        __FILE__,
                        __LINE__
                    );
                return cursor_cast_element<ValueType>(source.get_value(0));
            }
        }

    } // namespace detail

    /** Read-only provenance associated with the value selected by a cursor. */
    struct Provenance {
        std::string source_name;
        size_t source_line = 0;
        std::string source_code;
        ValueMetadata metadata;
        std::optional<SourceInfo> source;
    };

    class Cursor {
      protected:
        const Environment* env_;
        std::string path_;
        Path::Kind kind;

      public:
        explicit Cursor(const Environment* env, std::string_view path = "");

        /** Return cursors for named children when group traversal is implemented.
         * @return A map keyed by child name.
         * @note Group-child traversal is currently not implemented.
         */
        std::unordered_map<std::string, Cursor> children() const;

        /** Return cursors for elements of the current sequence node in index order.
         * @return A vector of element cursors; throws if the cursor is not a sequence.
         */
        std::vector<Cursor> elements() const;

        /** Return cursors for entries of the current mapping node.
         * @return A map keyed by entry name; throws if the cursor is not a mapping.
         */
        std::unordered_map<std::string, Cursor> items() const;

        /**
         * Test if keyed item exists for the current path
         *
         * @return true if item exists, false if not
         */
        bool has_item(const std::string& item) const;

        /**
         * Get current cursor path
         *
         * @return Fully qualified path of the cursor
         */
        const std::string& get_path() const;

        /** Return the value stored at this path.
         * @return A newly allocated VAL value; throws when the path is absent or not a value node.
         */
        val::BaseValue::PointerType get_value() const;

        /** Return the physical quantity metadata attached to the value at this path.
         * @return The quantity metadata, or `std::nullopt` when the node has no units.
         */
        std::optional<puq::Quantity> get_units() const;

        /**
         * Get a value node at the current cursor path
         *
         * @return A value node pointer
         */
        dip::ValueNode::PointerType get_node() const;

        /**
         * Return source and citation provenance for the value at this path.
         *
         * The source manifest entry is present when the environment has a
         * registered source (after parsing) or a DIPH5 v2 source manifest.
         */
        Provenance get_provenance() const;

        /**
         * Get shape of a value node at the current cursor path
         *
         * @return Shape array of a value node
         */
        val::Array::ShapeType get_shape() const;

        /**
         * Get path kind of the current cursor
         *
         * @return Cursor path kind
         */
        const Path::Kind get_kind() const;

        /**
         * Get string representation of the cursor
         *
         * @return String representation of a cursor
         */
        const std::string to_string() const;

        /**
         * Cast cursor as a specific scalar/array/vector type
         *
         * @tparam Expected value type
         */
        template <typename T> T as() const {
            auto value = env_->request_value("?" + path_);
            if (const auto* source = dynamic_cast<val::ArrayValue<uint8_t>*>(value.get()))
                return detail::cursor_cast_value<T>(*source, path_);
            if (const auto* source = dynamic_cast<val::ArrayValue<int64_t>*>(value.get()))
                return detail::cursor_cast_value<T>(*source, path_);
            if (const auto* source = dynamic_cast<val::ArrayValue<uint64_t>*>(value.get()))
                return detail::cursor_cast_value<T>(*source, path_);
            if (const auto* source = dynamic_cast<val::ArrayValue<double>*>(value.get()))
                return detail::cursor_cast_value<T>(*source, path_);
            if (const auto* source = dynamic_cast<val::ArrayValue<long double>*>(value.get()))
                return detail::cursor_cast_value<T>(*source, path_);
            if (const auto* source = dynamic_cast<val::ArrayValue<std::string>*>(value.get()))
                return detail::cursor_cast_value<T>(*source, path_);
            detail::throw_cursor_type_mismatch(path_);
        };

        /**
         * Get cursor from a fully qualified path, its part, or a keyed item from a map collection
         *
         * @param path Relative path, map key, or child name.
         * @return Cursor at the given path
         */
        Cursor operator[](std::string_view path) const;

        /**
         * Select indexed item from a list collection
         *
         * @param index Zero-based list index.
         * @return Item cursor at the given index
         */
        Cursor operator[](size_t index) const;
    };

} // namespace snt::dip

#endif // DIP_CURSOR_H
