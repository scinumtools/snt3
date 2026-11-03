#include "writer_common.h"
#include "naming.h"

#include <snt/dip/exceptions.h>
#include <unordered_set>
#include <vector>

namespace snt::dip::generate {
    namespace {
        std::string julia_scalar_type(core::DataType type) {
            switch (type) {
            case core::DataType::Integer8:
                return "Int8";
            case core::DataType::Integer16:
                return "Int16";
            case core::DataType::Integer32:
                return "Int32";
            case core::DataType::Integer64:
                return "Int64";
            case core::DataType::Integer8_U:
                return "UInt8";
            case core::DataType::Integer16_U:
                return "UInt16";
            case core::DataType::Integer32_U:
                return "UInt32";
            case core::DataType::Integer64_U:
                return "UInt64";
            case core::DataType::Float32:
                return "Float32";
            case core::DataType::Float64:
                return "Float64";
            case core::DataType::Float128:
                return "BigFloat";
            default:
                return {};
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
                    while (position < node.literal.size() &&
                           std::isspace(static_cast<unsigned char>(node.literal[position])))
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

                    while (position < node.literal.size() &&
                           std::isspace(static_cast<unsigned char>(node.literal[position])))
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

    } // namespace

    void write_julia(std::ostream& output, const Node& root) {
        JuliaWriter(output).write(root);
    }
} // namespace snt::dip::generate
