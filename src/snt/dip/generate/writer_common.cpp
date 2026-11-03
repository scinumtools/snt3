#include "writer_common.h"

#include <cctype>
#include <snt/dip/exceptions.h>

namespace snt::dip::generate {
    std::string indent(size_t depth) { return std::string(depth * 2, ' '); }
    bool is_scalar(const Node& node) { return node.has_value && node.children.empty(); }
        std::string cpp_scalar_type(core::DataType type) {
            switch (type) {
            case core::DataType::Boolean:
                return "bool";
            case core::DataType::Integer8:
                return "std::int8_t";
            case core::DataType::Integer16:
                return "std::int16_t";
            case core::DataType::Integer32:
                return "std::int32_t";
            case core::DataType::Integer64:
                return "std::int64_t";
            case core::DataType::Integer8_U:
                return "std::uint8_t";
            case core::DataType::Integer16_U:
                return "std::uint16_t";
            case core::DataType::Integer32_U:
                return "std::uint32_t";
            case core::DataType::Integer64_U:
                return "std::uint64_t";
            case core::DataType::Float32:
                return "float";
            case core::DataType::Float64:
                return "double";
            case core::DataType::Float128:
                return "long double";
            case core::DataType::String:
                return "std::string_view";
            default:
                throw dip::EnvironmentException(
                    "Unsupported export value",
                    "The value type cannot be exported to C++.",
                    "Use a supported scalar type.",
                    __FILE__,
                    __LINE__
                );
            }
        }

        std::string cpp_value_type(const Node& node) {
            std::string type = cpp_scalar_type(node.dtype);
            if (!node.is_array)
                return type;
            for (auto dimension = node.shape.rbegin(); dimension != node.shape.rend(); ++dimension)
                type = "std::array<" + type + ", " + std::to_string(*dimension) + ">";
            return type;
        }

        std::string cpp_literal(std::string literal) {
            bool quoted = false;
            bool escaped = false;
            for (char& character : literal) {
                if (quoted && escaped) {
                    escaped = false;
                    continue;
                }
                if (quoted && character == '\\') {
                    escaped = true;
                    continue;
                }
                if (character == '"') {
                    quoted = !quoted;
                    continue;
                }
                if (!quoted && character == '[')
                    character = '{';
                else if (!quoted && character == ']')
                    character = '}';
            }
            return literal;
        }

        std::string cpp_initializer(const Node& node) {
            if (!node.is_array)
                return cpp_literal(node.literal);

            std::string output;
            bool quoted = false;
            bool escaped = false;
            for (const char character : node.literal) {
                if (quoted && escaped) {
                    output += character;
                    escaped = false;
                    continue;
                }
                if (quoted && character == '\\') {
                    output += character;
                    escaped = true;
                    continue;
                }
                if (character == '"') {
                    output += character;
                    quoted = !quoted;
                    continue;
                }
                if (!quoted && character == '[')
                    output += "{{";
                else if (!quoted && character == ']')
                    output += "}}";
                else
                    output += character;
            }
            return output;
        }

        std::string cpp_escape(const std::string& value) {
            std::string output;
            output.reserve(value.size());
            for (const char character : value) {
                if (character == '\\' || character == '"')
                    output += '\\';
                output += character;
            }
            return output;
        }

        std::string type_stem(const std::string& path) {
            std::string output;
            bool capitalize = true;
            for (const unsigned char character : path) {
                if (std::isalnum(character) || character == '_') {
                    output += capitalize ? static_cast<char>(std::toupper(character)) : static_cast<char>(character);
                    capitalize = false;
                } else {
                    capitalize = true;
                }
            }
            return output.empty() ? "Parameters" : output;
        }

        std::string collection_path(const Node& node) {
            const size_t bracket = node.path.rfind('[');
            return bracket == std::string::npos ? node.path : node.path.substr(0, bracket);
        }

        std::string cpp_object_type(const Node& node) {
            switch (node.kind) {
            case NodeKind::Root:
                return "Parameters";
            case NodeKind::MapItem:
                return type_stem(collection_path(node)) + "Value";
            case NodeKind::ListItem:
                return type_stem(collection_path(node)) + "Item";
            default:
                return type_stem(node.path);
            }
        }

        std::string cpp_item_type(const Node& item) {
            return is_scalar(item) ? cpp_value_type(item) : cpp_object_type(item);
        }

        bool same_shape(const Node& left, const Node& right) {
            const bool compare_name = left.kind != NodeKind::MapItem && left.kind != NodeKind::ListItem;
            if (left.kind != right.kind || (compare_name && left.name != right.name) ||
                left.has_value != right.has_value || left.dtype != right.dtype || left.shape != right.shape ||
                left.is_array != right.is_array || left.children.size() != right.children.size())
                return false;
            for (size_t index = 0; index < left.children.size(); ++index) {
                if (!same_shape(*left.children[index], *right.children[index]))
                    return false;
            }
            return true;
        }

} // namespace snt::dip::generate
