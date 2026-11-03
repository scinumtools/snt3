#include "writer_common.h"
#include "naming.h"

#include <snt/dip/exceptions.h>
#include <unordered_set>
#include <vector>

namespace snt::dip::generate {
    namespace {
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

    } // namespace

    void write_c(std::ostream& output, const Node& root) {
        CWriter(output).write(root);
    }
} // namespace snt::dip::generate
