#ifndef DIP_CURSOR_H
#define DIP_CURSOR_H

#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/path.h>
#include <snt/val/value_base.h>
#include <snt/val/values_number.h>
#include <snt/val/values_string.h>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace snt::dip {

    class Cursor {
      protected:
        const Environment* env_;
        std::string path_;
        Path::Kind kind;

      public:
        explicit Cursor(const Environment* env, std::string_view path = "");

        /**
         * Get children nodes
         *
         * @return Map of children node cursors
         */
        std::unordered_map<std::string, Cursor> children() const;

        /**
         * Get list elements
         *
         * @return Vector of children cursors
         */
        std::vector<Cursor> elements() const;

        /**
         * Get map items
         *
         * @return Map of children cursors
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

        /**
         * Get value node at the current cursor path
         *
         * @return Pointer to a value node
         */
        val::BaseValue::PointerType get_value() const;

        /**
         * Get units of a value node at the current cursor path
         *
         * @return Units of a value node
         */
        std::optional<puq::Quantity> get_units() const;

        /**
         * Get a value node at the current cursor path
         *
         * @return A value node pointer
         */
        dip::ValueNode::PointerType get_node() const;

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
            using ValueType = detail::element_type_t<T>;
            using StorageType = detail::storage_type_t<ValueType>;
            auto value = env_->request_value("?" + path_);
            auto* val = dynamic_cast<val::ArrayValue<StorageType>*>(value.get());
            if (!val)
                throw dip::SyntaxException(
                    "Type mismatch",
                    "The value of node `" + path_ + "` cannot be converted to the requested type.",
                    "Request the value using a type that matches the node's data type.",
                    __FILE__,
                    __LINE__
                );
            if constexpr (detail::is_std_vector_v<T>) {
                if constexpr (std::is_same_v<ValueType, bool>) {
                    std::vector<bool> result;
                    result.reserve(val->get_size());
                    for (auto v : val->get_values())
                        result.push_back(v != 0);
                    return result;
                } else {
                    return val->get_values();
                }
            } else if constexpr (detail::is_std_array_v<T>) {
                auto values = val->get_values();
                if (values.size() != std::tuple_size_v<T>)
                    throw dip::EnvironmentException(
                        "Array size mismatch",
                        "The value of node `" + path_ + "` contains " + std::to_string(values.size()) +
                            " elements, but the requested array contains " + std::to_string(std::tuple_size_v<T>) +
                            " elements.",
                        "Request an array with the same number of elements as the node value.",
                        __FILE__,
                        __LINE__
                    );
                T result;
                if constexpr (std::is_same_v<ValueType, bool>) {
                    std::transform(values.begin(), values.end(), result.begin(), [](uint8_t v) { return v != 0; });
                } else {
                    std::copy(values.begin(), values.end(), result.begin());
                }
                return result;
            } else {
                if (val->get_size() != 1)
                    throw dip::SyntaxException(
                        "Expected scalar value",
                        "The value of node `" + path_ + "` contains " + std::to_string(val->get_size()) +
                            " elements, but a scalar value was requested.",
                        "Request a scalar node value or use a vector or array type for multiple elements.",
                        __FILE__,
                        __LINE__
                    );
                if constexpr (std::is_same_v<ValueType, bool>)
                    return val->get_value(0) != 0;
                else
                    return val->get_value(0);
            }
        };

        /**
         * Get cursor from a fully qualified path, its part, or a keyed item from a map collection
         *
         * @return Cursor at the given path
         */
        Cursor operator[](std::string_view path) const;

        /**
         * Select indexed item from a list collection
         *
         * @return Item cursor at the given index
         */
        Cursor operator[](size_t index) const;
    };

} // namespace snt::dip

#endif // DIP_CURSOR_H
