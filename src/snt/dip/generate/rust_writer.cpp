#include "writer_common.h"
#include "naming.h"

#include <snt/dip/exceptions.h>
#include <unordered_set>
#include <vector>

namespace snt::dip::generate {
    namespace {
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
                if (!node.is_array)
                    return result;
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

    } // namespace

    void write_rust(std::ostream& output, const Node& root) {
        RustWriter(output).write(root);
    }
} // namespace snt::dip::generate
