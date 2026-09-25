#include "export.h"

#include "model.h"
#include "naming.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <snt/dip/exceptions.h>
#include <unordered_map>
#include <vector>

namespace snt::dip::generate {
    namespace {
        std::string indent(size_t depth) {
            return std::string(depth * 2, ' ');
        }

        bool is_scalar(const Node& node) {
            return node.has_value && node.children.empty();
        }

        bool has_embedded_value(const Node& node) {
            return node.has_value && !node.children.empty();
        }

        void write_json(std::ostream& output, const Node& node, size_t depth) {
            if (node.kind == NodeKind::List) {
                output << '[';
                for (size_t index = 0; index < node.children.size(); ++index) {
                    if (index)
                        output << ',';
                    output << '\n' << indent(depth + 1);
                    write_json(output, *node.children[index], depth + 1);
                }
                if (!node.children.empty())
                    output << '\n' << indent(depth);
                output << ']';
                return;
            }
            if (is_scalar(node)) {
                output << node.literal;
                return;
            }

            output << '{';
            const size_t value_offset = has_embedded_value(node) ? 1 : 0;
            if (has_embedded_value(node))
                output << '\n' << indent(depth + 1) << "\"$value\": " << node.literal;
            for (size_t index = 0; index < node.children.size(); ++index) {
                if (index + value_offset)
                    output << ',';
                const Node& child = *node.children[index];
                output << '\n' << indent(depth + 1) << '"' << child.name << "\": ";
                write_json(output, child, depth + 1);
            }
            if (has_embedded_value(node) || !node.children.empty())
                output << '\n' << indent(depth);
            output << '}';
        }

        void write_yaml(std::ostream& output, const Node& node, size_t depth) {
            if (is_scalar(node)) {
                output << node.literal;
                return;
            }
            if (node.kind == NodeKind::List) {
                for (const auto& child : node.children) {
                    output << indent(depth) << "- ";
                    if (is_scalar(*child)) {
                        write_yaml(output, *child, depth + 1);
                        output << '\n';
                    } else {
                        output << '\n';
                        write_yaml(output, *child, depth + 1);
                    }
                }
                return;
            }
            if (has_embedded_value(node))
                output << indent(depth) << "$value: " << node.literal << '\n';
            for (const auto& child : node.children) {
                output << indent(depth) << child->name << ':';
                if (is_scalar(*child)) {
                    output << ' ';
                    write_yaml(output, *child, depth + 1);
                    output << '\n';
                } else {
                    output << '\n';
                    write_yaml(output, *child, depth + 1);
                }
            }
        }

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

        class CppWriter {
          public:
            explicit CppWriter(std::ostream& output) : output_(output) {}

            void write(const Node& root) {
                write_definitions(root);
                output_ << "inline constexpr Parameters parameters = " << initializer(root) << ";\n";
            }

          private:
            std::ostream& output_;
            std::unordered_set<std::string> emitted_types_;
            std::unordered_map<std::string, std::string> type_paths_;

            void register_type(const std::string& type, const std::string& path) {
                const auto found = type_paths_.find(type);
                if (found == type_paths_.end()) {
                    type_paths_.emplace(type, path);
                    return;
                }
                if (found->second != path)
                    throw dip::EnvironmentException(
                        "Export name collision",
                        "The DIPL paths `" + found->second + "` and `" + path + "` map to the same C++ type name `" +
                            type + "`.",
                        "Rename one of the colliding DIPL nodes before exporting.",
                        __FILE__,
                        __LINE__
                    );
            }

            void validate_collection(const Node& collection) const {
                if (collection.children.empty())
                    return;
                const Node& prototype = *collection.children.front();
                for (size_t index = 1; index < collection.children.size(); ++index) {
                    if (!same_shape(prototype, *collection.children[index]))
                        throw dip::EnvironmentException(
                            "Non-homogeneous collection export",
                            "The collection `" + collection.path + "` has entries with different shapes.",
                            "Use entries with the same fields and value types before exporting to C++.",
                            __FILE__,
                            __LINE__
                        );
                }
            }

            std::string type(const Node& node) const {
                if (is_scalar(node))
                    return cpp_value_type(node);
                if (node.kind == NodeKind::Map)
                    return "std::array<" + type_stem(node.path) + "Entry, " + std::to_string(node.children.size()) +
                           ">";
                if (node.kind == NodeKind::List)
                    return "std::array<" + cpp_item_type(*node.children.front()) + ", " +
                           std::to_string(node.children.size()) + ">";
                return cpp_object_type(node);
            }

            void write_definitions(const Node& node) {
                if (is_scalar(node))
                    return;
                if (node.kind == NodeKind::Map || node.kind == NodeKind::List) {
                    validate_collection(node);
                    if (!node.children.empty())
                        write_definitions(*node.children.front());
                    if (node.kind == NodeKind::Map)
                        write_map_entry(node);
                    return;
                }
                for (const auto& child : node.children)
                    write_definitions(*child);
                write_object(node);
            }

            void write_map_entry(const Node& map) {
                const std::string type_name = type_stem(map.path) + "Entry";
                register_type(type_name, map.path + "[]");
                if (!emitted_types_.insert(type_name).second)
                    return;
                output_ << "struct " << type_name << " {\n"
                        << "  std::string_view key;\n"
                        << "  " << cpp_item_type(*map.children.front()) << " value;\n"
                        << "};\n\n";
            }

            void write_object(const Node& node) {
                const std::string type_name = cpp_object_type(node);
                register_type(type_name, node.kind == NodeKind::Root ? "" : node.path);
                if (!emitted_types_.insert(type_name).second)
                    return;
                output_ << "struct " << type_name << " {\n";
                std::unordered_set<std::string> used;
                for (const auto& child : node.children)
                    output_ << "  " << type(*child) << ' ' << identifier(child->name, used) << ";\n";
                output_ << "};\n\n";
            }

            std::string initializer(const Node& node) const {
                if (is_scalar(node))
                    return cpp_initializer(node);
                std::string output = "{ ";
                if (node.kind == NodeKind::Map) {
                    output += "{ ";
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        const Node& item = *node.children[index];
                        output += "{ \"" + cpp_escape(item.name) + "\", " + initializer(item) + " }";
                    }
                    return output + " } }";
                }
                if (node.kind == NodeKind::List) {
                    output += "{ ";
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        output += initializer(*node.children[index]);
                    }
                    return output + " } }";
                }
                for (size_t index = 0; index < node.children.size(); ++index) {
                    if (index)
                        output += ", ";
                    output += initializer(*node.children[index]);
                }
                return output + " }";
            }
        };

        std::string c_scalar_type(core::DataType type) {
            switch (type) {
            case core::DataType::Boolean:
                return "bool";
            case core::DataType::Integer8:
                return "int8_t";
            case core::DataType::Integer16:
                return "int16_t";
            case core::DataType::Integer32:
                return "int32_t";
            case core::DataType::Integer64:
                return "int64_t";
            case core::DataType::Integer8_U:
                return "uint8_t";
            case core::DataType::Integer16_U:
                return "uint16_t";
            case core::DataType::Integer32_U:
                return "uint32_t";
            case core::DataType::Integer64_U:
                return "uint64_t";
            case core::DataType::Float32:
                return "float";
            case core::DataType::Float64:
                return "double";
            case core::DataType::Float128:
                return "long double";
            case core::DataType::String:
                return "const char*";
            default:
                throw dip::EnvironmentException(
                    "Unsupported export value",
                    "The value type cannot be exported to C.",
                    "Use a supported scalar type.",
                    __FILE__,
                    __LINE__
                );
            }
        }

        class CWriter {
          public:
            explicit CWriter(std::ostream& output) : output_(output) {}

            void write(const Node& root) {
                write_definitions(root);
                output_ << "static const Parameters snt_parameters = " << initializer(root) << ";\n";
            }

          private:
            std::ostream& output_;
            std::unordered_set<std::string> emitted_types_;

            std::string object_type(const Node& node) const { return cpp_object_type(node); }

            std::string item_type(const Node& item) const {
                return is_scalar(item) ? c_scalar_type(item.dtype) : object_type(item);
            }

            void validate_collection(const Node& collection) const {
                if (collection.children.empty())
                    return;
                for (size_t index = 1; index < collection.children.size(); ++index) {
                    if (!same_shape(*collection.children.front(), *collection.children[index]))
                        throw dip::EnvironmentException(
                            "Non-homogeneous collection export",
                            "The collection `" + collection.path + "` has entries with different shapes.",
                            "Use entries with the same fields and value types before exporting to C.",
                            __FILE__,
                            __LINE__
                        );
                }
            }

            void write_member(const Node& node, const std::string& name) {
                if (is_scalar(node)) {
                    output_ << "  " << c_scalar_type(node.dtype) << ' ' << name;
                    for (const size_t dimension : node.shape)
                        output_ << '[' << dimension << ']';
                    output_ << ";\n";
                    return;
                }
                if (node.kind == NodeKind::Map) {
                    output_ << "  " << type_stem(node.path) << "Entry " << name << '[' << node.children.size()
                            << "];\n";
                    return;
                }
                if (node.kind == NodeKind::List) {
                    const Node& item = *node.children.front();
                    if (is_scalar(item)) {
                        output_ << "  " << c_scalar_type(item.dtype) << ' ' << name << '[' << node.children.size()
                                << ']';
                        for (const size_t dimension : item.shape)
                            output_ << '[' << dimension << ']';
                        output_ << ";\n";
                    } else {
                        output_ << "  " << item_type(item) << ' ' << name << '[' << node.children.size() << "];\n";
                    }
                    return;
                }
                output_ << "  " << object_type(node) << ' ' << name << ";\n";
            }

            void write_definitions(const Node& node) {
                if (is_scalar(node))
                    return;
                if (node.kind == NodeKind::Map || node.kind == NodeKind::List) {
                    validate_collection(node);
                    if (!node.children.empty())
                        write_definitions(*node.children.front());
                    if (node.kind == NodeKind::Map)
                        write_map_entry(node);
                    return;
                }
                for (const auto& child : node.children)
                    write_definitions(*child);
                write_object(node);
            }

            void write_map_entry(const Node& map) {
                const std::string name = type_stem(map.path) + "Entry";
                if (!emitted_types_.insert(name).second)
                    return;
                output_ << "typedef struct " << name << " {\n"
                        << "  const char* key;\n";
                const Node& item = *map.children.front();
                if (is_scalar(item)) {
                    output_ << "  " << c_scalar_type(item.dtype) << " value";
                    for (const size_t dimension : item.shape)
                        output_ << '[' << dimension << ']';
                    output_ << ";\n";
                } else {
                    output_ << "  " << item_type(item) << " value;\n";
                }
                output_ << "} " << name << ";\n\n";
            }

            void write_object(const Node& node) {
                const std::string name = object_type(node);
                if (!emitted_types_.insert(name).second)
                    return;
                output_ << "typedef struct " << name << " {\n";
                std::unordered_set<std::string> used;
                for (const auto& child : node.children)
                    write_member(*child, identifier(child->name, used));
                output_ << "} " << name << ";\n\n";
            }

            std::string initializer(const Node& node) const {
                if (is_scalar(node))
                    return cpp_literal(node.literal);
                std::string output = "{ ";
                if (node.kind == NodeKind::Map) {
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        const Node& item = *node.children[index];
                        output += "{ \"" + cpp_escape(item.name) + "\", " + initializer(item) + " }";
                    }
                    return output + " }";
                }
                if (node.kind == NodeKind::List) {
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        output += initializer(*node.children[index]);
                    }
                    return output + " }";
                }
                for (size_t index = 0; index < node.children.size(); ++index) {
                    if (index)
                        output += ", ";
                    output += initializer(*node.children[index]);
                }
                return output + " }";
            }
        };

        std::string rust_scalar_type(core::DataType type) {
            switch (type) {
            case core::DataType::Boolean:
                return "bool";
            case core::DataType::Integer8:
                return "i8";
            case core::DataType::Integer16:
                return "i16";
            case core::DataType::Integer32:
                return "i32";
            case core::DataType::Integer64:
                return "i64";
            case core::DataType::Integer8_U:
                return "u8";
            case core::DataType::Integer16_U:
                return "u16";
            case core::DataType::Integer32_U:
                return "u32";
            case core::DataType::Integer64_U:
                return "u64";
            case core::DataType::Float32:
                return "f32";
            case core::DataType::Float64:
                return "f64";
            case core::DataType::Float128:
                throw dip::EnvironmentException(
                    "Unsupported export value",
                    "Rust has no portable native 128-bit floating-point type.",
                    "Use a 32-bit or 64-bit float before exporting to Rust.",
                    __FILE__,
                    __LINE__
                );
            case core::DataType::String:
                return "&'static str";
            default:
                throw dip::EnvironmentException(
                    "Unsupported export value",
                    "The value type cannot be exported to Rust.",
                    "Use a supported scalar type.",
                    __FILE__,
                    __LINE__
                );
            }
        }

        class RustWriter {
          public:
            explicit RustWriter(std::ostream& output) : output_(output) {}

            void write(const Node& root) {
                write_definitions(root);
                output_ << "pub static PARAMETERS: Parameters = " << initializer(root) << ";\n";
            }

          private:
            std::ostream& output_;
            std::unordered_set<std::string> emitted_types_;

            std::string item_type(const Node& item) const {
                return is_scalar(item) ? rust_value_type(item) : cpp_object_type(item);
            }

            std::string rust_value_type(const Node& node) const {
                std::string result = rust_scalar_type(node.dtype);
                for (auto dimension = node.shape.rbegin(); dimension != node.shape.rend(); ++dimension)
                    result = "[" + result + "; " + std::to_string(*dimension) + "]";
                return result;
            }

            std::string type(const Node& node) const {
                if (is_scalar(node))
                    return rust_value_type(node);
                if (node.kind == NodeKind::Map)
                    return "[" + type_stem(node.path) + "Entry; " + std::to_string(node.children.size()) + "]";
                if (node.kind == NodeKind::List)
                    return "[" + item_type(*node.children.front()) + "; " + std::to_string(node.children.size()) + "]";
                return cpp_object_type(node);
            }

            void validate_collection(const Node& collection) const {
                if (collection.children.empty())
                    return;
                for (size_t index = 1; index < collection.children.size(); ++index) {
                    if (!same_shape(*collection.children.front(), *collection.children[index]))
                        throw dip::EnvironmentException(
                            "Non-homogeneous collection export",
                            "The collection `" + collection.path + "` has entries with different shapes.",
                            "Use entries with the same fields and value types before exporting to Rust.",
                            __FILE__,
                            __LINE__
                        );
                }
            }

            void write_definitions(const Node& node) {
                if (is_scalar(node))
                    return;
                if (node.kind == NodeKind::Map || node.kind == NodeKind::List) {
                    validate_collection(node);
                    if (!node.children.empty())
                        write_definitions(*node.children.front());
                    if (node.kind == NodeKind::Map)
                        write_map_entry(node);
                    return;
                }
                for (const auto& child : node.children)
                    write_definitions(*child);
                write_object(node);
            }

            void write_map_entry(const Node& map) {
                const std::string name = type_stem(map.path) + "Entry";
                if (!emitted_types_.insert(name).second)
                    return;
                output_ << "pub struct " << name << " {\n"
                        << "    pub key: &'static str,\n"
                        << "    pub value: " << item_type(*map.children.front()) << ",\n"
                        << "}\n\n";
            }

            void write_object(const Node& node) {
                const std::string name = cpp_object_type(node);
                if (!emitted_types_.insert(name).second)
                    return;
                output_ << "pub struct " << name << " {\n";
                std::unordered_set<std::string> used;
                for (const auto& child : node.children)
                    output_ << "    pub " << identifier(child->name, used) << ": " << type(*child) << ",\n";
                output_ << "}\n\n";
            }

            std::string initializer(const Node& node) const {
                if (is_scalar(node))
                    return node.literal;
                if (node.kind == NodeKind::Map) {
                    std::string output = "[";
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        const Node& item = *node.children[index];
                        output += type_stem(node.path) + "Entry { key: \"" + cpp_escape(item.name) +
                                  "\", value: " + initializer(item) + " }";
                    }
                    return output + "]";
                }
                if (node.kind == NodeKind::List) {
                    std::string output = "[";
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        output += initializer(*node.children[index]);
                    }
                    return output + "]";
                }
                std::string output = cpp_object_type(node) + " { ";
                std::unordered_set<std::string> used;
                for (size_t index = 0; index < node.children.size(); ++index) {
                    if (index)
                        output += ", ";
                    const Node& child = *node.children[index];
                    output += identifier(child.name, used) + ": " + initializer(child);
                }
                return output + " }";
            }
        };

        std::string julia_scalar_type(core::DataType type) {
            switch (type) {
            case core::DataType::Integer8: return "Int8";
            case core::DataType::Integer16: return "Int16";
            case core::DataType::Integer32: return "Int32";
            case core::DataType::Integer64: return "Int64";
            case core::DataType::Integer8_U: return "UInt8";
            case core::DataType::Integer16_U: return "UInt16";
            case core::DataType::Integer32_U: return "UInt32";
            case core::DataType::Integer64_U: return "UInt64";
            case core::DataType::Float32: return "Float32";
            case core::DataType::Float64: return "Float64";
            case core::DataType::Float128: return "BigFloat";
            default: return {};
            }
        }

        std::string trim(std::string value) {
            const auto first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
                return {};
            const auto last = value.find_last_not_of(" \t\r\n");
            return value.substr(first, last - first + 1);
        }

        std::string julia_scalar_literal(const Node& node, const std::string& literal) {
            const std::string type = julia_scalar_type(node.dtype);
            if (type.empty())
                return literal;
            if (node.dtype == core::DataType::Float128)
                return "BigFloat(\"" + cpp_escape(literal) + "\")";
            return type + "(" + literal + ")";
        }

        class JuliaWriter {
          public:
            explicit JuliaWriter(std::ostream& output) : output_(output) {}

            void write(const Node& root) {
                output_ << "const parameters = ";
                write_initializer(root, 0);
                output_ << '\n';
            }

          private:
            std::ostream& output_;

            void write_array(const Node& node, size_t& position, size_t depth) {
                ++position; // Opening '['.
                output_ << '(';
                bool has_values = false;
                while (position < node.literal.size()) {
                    while (position < node.literal.size() && std::isspace(static_cast<unsigned char>(node.literal[position])))
                        ++position;
                    if (position == node.literal.size() || node.literal[position] == ']') {
                        if (position < node.literal.size())
                            ++position;
                        break;
                    }

                    output_ << '\n' << indent(depth + 1);
                    if (node.literal[position] == '[') {
                        write_array(node, position, depth + 1);
                    } else {
                        std::string token;
                        bool quoted = false;
                        bool escaped = false;
                        while (position < node.literal.size()) {
                            const char character = node.literal[position];
                            if (quoted && escaped) {
                                token += character;
                                escaped = false;
                                ++position;
                                continue;
                            }
                            if (quoted && character == '\\') {
                                token += character;
                                escaped = true;
                                ++position;
                                continue;
                            }
                            if (character == '"')
                                quoted = !quoted;
                            if (!quoted && (character == ',' || character == ']'))
                                break;
                            token += character;
                            ++position;
                        }
                        output_ << julia_scalar_literal(node, trim(token));
                    }
                    has_values = true;
                    output_ << ',';

                    while (position < node.literal.size() && std::isspace(static_cast<unsigned char>(node.literal[position])))
                        ++position;
                    if (position < node.literal.size() && node.literal[position] == ',')
                        ++position;
                }
                if (has_values)
                    output_ << '\n' << indent(depth);
                output_ << ')';
            }

            void write_initializer(const Node& node, size_t depth) {
                if (is_scalar(node)) {
                    if (node.is_array) {
                        size_t position = 0;
                        write_array(node, position, depth);
                    } else {
                        output_ << julia_scalar_literal(node, node.literal);
                    }
                    return;
                }

                if (node.kind == NodeKind::Map) {
                    output_ << '(';
                    for (const auto& item : node.children) {
                        output_ << '\n' << indent(depth + 1) << '"' << cpp_escape(item->name) << "\" => ";
                        write_initializer(*item, depth + 1);
                        output_ << ',';
                    }
                    if (!node.children.empty())
                        output_ << '\n' << indent(depth);
                    output_ << ')';
                    return;
                }

                if (node.kind == NodeKind::List) {
                    output_ << '(';
                    for (const auto& item : node.children) {
                        output_ << '\n' << indent(depth + 1);
                        write_initializer(*item, depth + 1);
                        output_ << ',';
                    }
                    if (!node.children.empty())
                        output_ << '\n' << indent(depth);
                    output_ << ')';
                    return;
                }

                output_ << "(;";
                std::unordered_set<std::string> used;
                for (const auto& child : node.children) {
                    output_ << '\n' << indent(depth + 1) << identifier(child->name, used) << " = ";
                    write_initializer(*child, depth + 1);
                    output_ << ',';
                }
                if (!node.children.empty())
                    output_ << '\n' << indent(depth);
                output_ << ')';
            }
        };

        std::string fortran_scalar_type(const Node& node) {
            switch (node.dtype) {
            case core::DataType::Boolean:
                return "logical";
            case core::DataType::Integer8:
                return "integer(int8)";
            case core::DataType::Integer16:
                return "integer(int16)";
            case core::DataType::Integer32:
                return "integer(int32)";
            case core::DataType::Integer64:
                return "integer(int64)";
            case core::DataType::Integer8_U:
            case core::DataType::Integer16_U:
            case core::DataType::Integer32_U:
            case core::DataType::Integer64_U:
                throw dip::EnvironmentException(
                    "Unsupported export value",
                    "Unsigned integers cannot be represented portably in Fortran.",
                    "Use a signed integer type before exporting to Fortran.",
                    __FILE__,
                    __LINE__
                );
            case core::DataType::Float32:
                return "real(real32)";
            case core::DataType::Float64:
                return "real(real64)";
            case core::DataType::Float128:
                return "real(real128)";
            case core::DataType::String: {
                const size_t quoted_size = node.literal.size() >= 2 ? node.literal.size() - 2 : 1;
                return "character(len=" + std::to_string(std::max<size_t>(quoted_size, 1)) + ")";
            }
            default:
                throw dip::EnvironmentException(
                    "Unsupported export value",
                    "The value type cannot be exported to Fortran.",
                    "Use a supported scalar type.",
                    __FILE__,
                    __LINE__
                );
            }
        }

        std::string fortran_dimensions(const std::vector<size_t>& shape) {
            if (shape.empty())
                return {};
            std::string output = ", dimension(";
            for (size_t index = 0; index < shape.size(); ++index) {
                if (index)
                    output += ", ";
                output += std::to_string(shape[index]);
            }
            return output + ')';
        }

        std::string fortran_literal(const Node& node) {
            if (node.literal == "true")
                return ".true.";
            if (node.literal == "false")
                return ".false.";
            return node.literal;
        }

        std::string fortran_array_literal(const Node& node) {
            if (node.shape.size() < 2)
                return fortran_literal(node);
            std::string values;
            bool quoted = false;
            bool escaped = false;
            for (const char character : node.literal) {
                if (quoted && escaped) {
                    values += character;
                    escaped = false;
                    continue;
                }
                if (quoted && character == '\\') {
                    values += character;
                    escaped = true;
                    continue;
                }
                if (character == '"') {
                    values += character;
                    quoted = !quoted;
                    continue;
                }
                if (!quoted && (character == '[' || character == ']'))
                    continue;
                values += character;
            }
            std::string shape = "[";
            std::string order = "[";
            for (size_t index = 0; index < node.shape.size(); ++index) {
                if (index) {
                    shape += ", ";
                    order += ", ";
                }
                shape += std::to_string(node.shape[index]);
                order += std::to_string(node.shape.size() - index);
            }
            return "reshape([" + values + "], " + shape + "], order=" + order + "])";
        }

        class FortranWriter {
          public:
            explicit FortranWriter(std::ostream& output) : output_(output) {}

            void write(const Node& root) {
                output_ << "module snt_parameters\n  use, intrinsic :: iso_fortran_env\n  implicit none\n  private\n  "
                           "public :: parameters\n\n";
                write_definitions(root);
                output_ << "  type(ParametersData), parameter :: parameters = " << initializer(root)
                        << "\nend module snt_parameters\n";
            }

          private:
            std::ostream& output_;
            std::unordered_set<std::string> emitted_types_;

            std::string object_type(const Node& node) const {
                return node.kind == NodeKind::Root ? "ParametersData" : cpp_object_type(node);
            }

            std::string item_type(const Node& item) const {
                return is_scalar(item) ? fortran_scalar_type(item) : object_type(item);
            }

            void validate_collection(const Node& collection) const {
                if (collection.children.empty())
                    return;
                for (size_t index = 1; index < collection.children.size(); ++index) {
                    if (!same_shape(*collection.children.front(), *collection.children[index]))
                        throw dip::EnvironmentException(
                            "Non-homogeneous collection export",
                            "The collection `" + collection.path + "` has entries with different shapes.",
                            "Use entries with the same fields and value types before exporting to Fortran.",
                            __FILE__,
                            __LINE__
                        );
                }
            }

            void write_member(const Node& node, const std::string& name) {
                if (is_scalar(node)) {
                    output_ << "    " << fortran_scalar_type(node) << fortran_dimensions(node.shape) << " :: " << name
                            << "\n";
                    return;
                }
                if (node.kind == NodeKind::Map) {
                    output_ << "    type(" << type_stem(node.path) << "Entry), dimension(" << node.children.size()
                            << ") :: " << name << "\n";
                    return;
                }
                if (node.kind == NodeKind::List) {
                    const Node& item = *node.children.front();
                    if (is_scalar(item)) {
                        std::vector<size_t> dimensions{node.children.size()};
                        dimensions.insert(dimensions.end(), item.shape.begin(), item.shape.end());
                        output_ << "    " << fortran_scalar_type(item) << fortran_dimensions(dimensions)
                                << " :: " << name << "\n";
                    } else
                        output_ << "    type(" << item_type(item) << "), dimension(" << node.children.size()
                                << ") :: " << name << "\n";
                    return;
                }
                output_ << "    type(" << object_type(node) << ") :: " << name << "\n";
            }

            void write_definitions(const Node& node) {
                if (is_scalar(node))
                    return;
                if (node.kind == NodeKind::Map || node.kind == NodeKind::List) {
                    validate_collection(node);
                    if (!node.children.empty())
                        write_definitions(*node.children.front());
                    if (node.kind == NodeKind::Map)
                        write_map_entry(node);
                    return;
                }
                for (const auto& child : node.children)
                    write_definitions(*child);
                write_object(node);
            }

            void write_map_entry(const Node& map) {
                const std::string name = type_stem(map.path) + "Entry";
                if (!emitted_types_.insert(name).second)
                    return;
                size_t key_length = 1;
                for (const auto& child : map.children)
                    key_length = std::max(key_length, child->name.size());
                output_ << "  type :: " << name << "\n"
                        << "    character(len=" << key_length << ") :: key\n";
                const Node& item = *map.children.front();
                if (is_scalar(item))
                    output_ << "    " << fortran_scalar_type(item) << fortran_dimensions(item.shape) << " :: value\n";
                else
                    output_ << "    type(" << item_type(item) << ") :: value\n";
                output_ << "  end type " << name << "\n\n";
            }

            void write_object(const Node& node) {
                const std::string name = object_type(node);
                if (!emitted_types_.insert(name).second)
                    return;
                output_ << "  type :: " << name << "\n";
                std::unordered_set<std::string> used;
                for (const auto& child : node.children)
                    write_member(*child, identifier(child->name, used));
                output_ << "  end type " << name << "\n\n";
            }

            std::string initializer(const Node& node) const {
                if (is_scalar(node))
                    return node.is_array ? fortran_array_literal(node) : fortran_literal(node);
                if (node.kind == NodeKind::Map) {
                    std::string output = "[";
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        const Node& item = *node.children[index];
                        output += type_stem(node.path) + "Entry(key=\"" + cpp_escape(item.name) +
                                  "\", value=" + initializer(item) + ")";
                    }
                    return output + "]";
                }
                if (node.kind == NodeKind::List) {
                    std::string output = "[";
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        output += initializer(*node.children[index]);
                    }
                    return output + "]";
                }
                std::string output = object_type(node) + "(";
                std::unordered_set<std::string> used;
                for (size_t index = 0; index < node.children.size(); ++index) {
                    if (index)
                        output += ", ";
                    const Node& child = *node.children[index];
                    output += identifier(child.name, used) + "=" + initializer(child);
                }
                return output + ")";
            }
        };
    } // namespace

    void write(const Environment& environment, ExportFormat format, const std::filesystem::path& file) {
        const std::unique_ptr<Node> model = build_model(environment);
        std::ofstream output(file, std::ios::binary | std::ios::trunc);
        if (!output)
            throw dip::IOException(
                "Unable to create exported environment",
                "The file `" + file.string() + "` could not be opened for writing.",
                "Check that its parent directory exists and is writable.",
                __FILE__,
                __LINE__
            );
        switch (format) {
        case ExportFormat::CPP:
            output << "#pragma once\n\n#include <array>\n#include <cstdint>\n#include <string_view>\n\nnamespace "
                      "parameters {\n";
            CppWriter(output).write(*model);
            output << "} // namespace parameters\n";
            break;
        case ExportFormat::C:
            output << "#pragma once\n\n#include <stdbool.h>\n#include <stdint.h>\n\n";
            CWriter(output).write(*model);
            break;
        case ExportFormat::RUST:
            RustWriter(output).write(*model);
            break;
        case ExportFormat::JULIA:
            JuliaWriter(output).write(*model);
            break;
        case ExportFormat::FORTRAN:
            FortranWriter(output).write(*model);
            break;
        case ExportFormat::JSON:
            write_json(output, *model, 0);
            output << '\n';
            break;
        case ExportFormat::YAML:
            write_yaml(output, *model, 0);
            break;
        default:
            throw dip::MissingException("This export format is not implemented yet.", __FILE__, __LINE__);
        }
        if (!output)
            throw dip::IOException(
                "Unable to write exported environment",
                "Writing the file `" + file.string() + "` failed.",
                "Check that the destination has sufficient free space and is writable.",
                __FILE__,
                __LINE__
            );
    }
} // namespace snt::dip::generate
