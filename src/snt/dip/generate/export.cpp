#include "export.h"

#include "../nodes/metadata_fields.h"
#include "model.h"
#include "naming.h"
#include "writer_common.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <snt/dip/cursor.h>
#include <snt/dip/exceptions.h>
#include <unordered_map>
#include <vector>

namespace snt::dip::generate {
    namespace {
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

        void write_json_string(std::ostream& output, const std::string& value) {
            output << '"';
            for (const unsigned char character : value) {
                switch (character) {
                case '"':
                    output << "\\\"";
                    break;
                case '\\':
                    output << "\\\\";
                    break;
                case '\b':
                    output << "\\b";
                    break;
                case '\f':
                    output << "\\f";
                    break;
                case '\n':
                    output << "\\n";
                    break;
                case '\r':
                    output << "\\r";
                    break;
                case '\t':
                    output << "\\t";
                    break;
                default:
                    if (character < 0x20) {
                        constexpr char hex[] = "0123456789abcdef";
                        output << "\\u00" << hex[character >> 4] << hex[character & 0x0f];
                    } else {
                        output << static_cast<char>(character);
                    }
                }
            }
            output << '"';
        }

        void write_string_list(std::ostream& output, const std::vector<std::string>& values) {
            output << '[';
            for (size_t index = 0; index < values.size(); ++index) {
                if (index)
                    output << ", ";
                write_json_string(output, values[index]);
            }
            output << ']';
        }

        void write_metadata_value(std::ostream& output, const ValueMetadata& metadata, const std::string& name) {
#define SNT_METADATA_JSON(TYPE, FIELD, NAME)                                                                           \
    if (name == #FIELD) {                                                                                              \
        write_json_string(output, metadata.FIELD);                                                                     \
        return;                                                                                                        \
    }
#define SNT_METADATA_JSON_LIST(TYPE, FIELD, NAME)                                                                      \
    if (name == #FIELD) {                                                                                              \
        write_string_list(output, metadata.FIELD);                                                                     \
        return;                                                                                                        \
    }
            SNT_DIP_METADATA_SCALAR_FIELDS(SNT_METADATA_JSON)
            SNT_DIP_METADATA_LIST_FIELDS(SNT_METADATA_JSON_LIST)
#undef SNT_METADATA_JSON_LIST
#undef SNT_METADATA_JSON
            output << "null";
        }

        void write_metadata(std::ostream& output, const ValueMetadata& metadata) {
            const auto names = metadata.present_names();
            output << "{\"present\": ";
            write_string_list(output, names);
            output << ", \"values\": {";
            for (size_t index = 0; index < names.size(); ++index) {
                if (index)
                    output << ", ";
                write_json_string(output, names[index]);
                output << ": ";
                write_metadata_value(output, metadata, names[index]);
            }
            output << "}}";
        }

        const Node* find_value_node(const Node& node, const std::string& path) {
            if (node.path == path && node.has_value)
                return &node;
            for (const auto& child : node.children)
                if (const auto* found = find_value_node(*child, path))
                    return found;
            return nullptr;
        }

        std::string path_leaf(const std::string& path) {
            const size_t separator = path.rfind('.');
            const size_t begin = separator == std::string::npos ? 0 : separator + 1;
            const size_t bracket = path.find('[', begin);
            return path.substr(begin, bracket == std::string::npos ? std::string::npos : bracket - begin);
        }

        void write_source_info(std::ostream& output, const SourceInfo& source) {
            output << "{\"path\": ";
            write_json_string(output, source.path);
            output << ", \"parent\": ";
            if (source.parent_name.empty())
                output << "null";
            else {
                output << "{\"name\": ";
                write_json_string(output, source.parent_name);
                output << ", \"line\": " << source.parent_line << '}';
            }
            output << ", \"hash_algorithm\": ";
            if (source.hash_algorithm.empty())
                output << "null";
            else
                write_json_string(output, source.hash_algorithm);
            output << ", \"hash\": ";
            if (source.hash.empty())
                output << "null";
            else
                write_json_string(output, source.hash);
            output << ", \"metadata\": ";
            write_metadata(output, source.metadata);
            output << '}';
        }

        const char* hierarchy_node_kind(const std::string& path, const Collection& collection,
                                        const std::unordered_map<std::string, Collection>& collections) {
            if (collection.kind == Path::Kind::Map)
                return "map";
            if (collection.kind == Path::Kind::List)
                return "list";
            if (collection.kind == Path::Kind::Group)
                return "group";
            if (collection.kind == Path::Kind::Item) {
                const size_t open = path.rfind('[');
                if (open != std::string::npos) {
                    const auto parent = collections.find(path.substr(0, open));
                    if (parent != collections.end() && parent->second.kind == Path::Kind::List)
                        return "list_item";
                }
                return "map_item";
            }
            return "node";
        }

        void write_node_metadata(std::ostream& output, const Environment& environment) {
            struct Record {
                std::string kind;
                const ValueMetadata* metadata = nullptr;
            };
            std::map<std::string, Record> records;
            const auto& collections = environment.hierarchy.get_collections();
            std::unordered_map<std::string, bool> table_paths;
            for (const auto& node : environment.nodes.get_nodes())
                if (node && !node->table_path.empty())
                    table_paths[node->table_path] = true;
            for (const auto& [path, collection] : collections) {
                if (!collection.metadata.present.empty()) {
                    const std::string kind = table_paths.count(path)
                                                 ? "table"
                                                 : hierarchy_node_kind(path, collection, collections);
                    records[path] = {kind, &collection.metadata};
                }
            }
            for (const auto& node : environment.nodes.get_nodes()) {
                if (node && !node->metadata.present.empty()) {
                    const std::string kind = node->table_path.empty() ? "value" : "table_column";
                    records[node->path.name] = {kind, &node->metadata};
                }
            }

            output << "{\n";
            bool first = true;
            for (const auto& [path, record] : records) {
                if (!first)
                    output << ",\n";
                first = false;
                output << "    ";
                write_json_string(output, path);
                output << ": {\"kind\": ";
                write_json_string(output, record.kind);
                output << ", \"metadata\": ";
                write_metadata(output, *record.metadata);
                output << '}';
            }
            if (!records.empty())
                output << '\n';
            output << "  }";
        }

        void write_json_metadata(std::ostream& output, const Environment& environment, const Node& model) {
            output << "{\n  \"schema\": \"snt-dip-values/2\",\n  \"values\": {";
            bool first_value = true;
            for (const auto& evaluated_node : environment.nodes.get_nodes()) {
                if (!evaluated_node || (!evaluated_node->value && evaluated_node->value_origin != ValueOrigin::None &&
                                        evaluated_node->dtype != NodeDtype::Link))
                    continue;
                const bool is_link = evaluated_node->dtype == NodeDtype::Link;
                const auto* model_node = find_value_node(model, evaluated_node->path.name);
                if (!model_node && evaluated_node->value_origin != ValueOrigin::None && !is_link)
                    continue;
                if (!first_value)
                    output << ",\n";
                else
                    output << '\n';
                first_value = false;
                const auto provenance = environment[evaluated_node->path.name].get_provenance();
                output << "    ";
                write_json_string(output, evaluated_node->path.name);
                output << ": {\"name\": ";
                write_json_string(output, model_node ? model_node->name : path_leaf(evaluated_node->path.name));
                output << ", \"type\": ";
                const auto type = core::DataTypeNames.find(evaluated_node->value_dtype);
                write_json_string(output, is_link ? "link" :
                                                type == core::DataTypeNames.end() ? "unknown" : type->second);
                output << ", \"shape\": [";
                const auto shape = model_node && model_node->is_array ? model_node->shape : std::vector<size_t>{};
                for (size_t dimension = 0; dimension < shape.size(); ++dimension) {
                    if (dimension)
                        output << ", ";
                    output << shape[dimension];
                }
                output << "], \"is_array\": " << (!evaluated_node->dimension.empty() ? "true" : "false")
                       << ", \"presence\": " << (is_link ? "\"link\"" :
                                                      evaluated_node->value ? "\"value\"" : "\"none\"")
                       << ", \"value\": " << (model_node ? model_node->literal : "null") << ", \"units\": ";
                if (evaluated_node->units)
                    write_json_string(output, evaluated_node->units->to_string());
                else
                    output << "null";
                if (is_link) {
                    output << ", \"link\": {\"source\": ";
                    write_json_string(output, evaluated_node->link_source);
                    output << ", \"target\": ";
                    write_json_string(output, evaluated_node->link_target);
                    output << '}';
                }
                output << ", \"metadata\": ";
                write_metadata(output, evaluated_node->metadata);
                output << ", \"provenance\": {\"source\": ";
                write_json_string(output, provenance.source_name);
                output << ", \"line\": " << provenance.source_line << ", \"code\": ";
                write_json_string(output, provenance.source_code);
                output << ", \"overridden\": " << (evaluated_node->override ? "true" : "false");
                if (evaluated_node->override) {
                    output << ", \"override\": {\"source\": ";
                    write_json_string(output, evaluated_node->override_line.source.name);
                    output << ", \"line\": " << evaluated_node->override_line.source.line_number << '}';
                }
                output << "}}";
            }
            if (!first_value)
                output << '\n' << "  ";
            output << "},\n  \"node_metadata\": ";
            write_node_metadata(output, environment);
            output << ",\n  \"sources\": {";
            const auto sources = environment.get_source_manifest();
            for (size_t index = 0; index < sources.size(); ++index) {
                if (index)
                    output << ',';
                output << '\n' << "    ";
                write_json_string(output, sources[index].name);
                output << ": ";
                write_source_info(output, sources[index]);
            }
            if (!sources.empty())
                output << '\n' << "  ";
            output << "}\n}\n";
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
            write_cpp(output, *model);
            output << "} // namespace parameters\n";
            break;
        case ExportFormat::C:
            output << "#pragma once\n\n#include <stdbool.h>\n#include <stdint.h>\n\n";
            write_c(output, *model);
            break;
        case ExportFormat::RUST:
            write_rust(output, *model);
            break;
        case ExportFormat::JULIA:
            write_julia(output, *model);
            break;
        case ExportFormat::FORTRAN:
            write_fortran(output, *model);
            break;
        case ExportFormat::JSON:
            write_json(output, *model, 0);
            output << '\n';
            break;
        case ExportFormat::JSON_METADATA:
            write_json_metadata(output, environment, *model);
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
