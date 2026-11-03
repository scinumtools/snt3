#include "writer_common.h"
#include "naming.h"

#include <snt/dip/exceptions.h>
#include <unordered_set>
#include <vector>

namespace snt::dip::generate {
    namespace {
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
                    output_ << "    " << fortran_scalar_type(node)
                            << (node.is_array ? fortran_dimensions(node.shape) : "") << " :: " << name << "\n";
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

    void write_fortran(std::ostream& output, const Node& root) {
        FortranWriter(output).write(root);
    }
} // namespace snt::dip::generate
