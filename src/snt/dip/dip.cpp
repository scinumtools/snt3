#include "parsers.h"

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_property.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/settings.h>
#include <sstream>
#include <string>

namespace snt::dip {

    namespace {

        constexpr std::string_view project_preamble = R"DIPL($schema snt_project_unit
  name str
  unit str
$schema snt_project_source
  name str
  filepath str
$schema snt_project_code
  file str = none
  string str = none
units list : snt_project_unit
sources list : snt_project_source
code list : snt_project_code
)DIPL";

        std::string top_level_name(const std::string& path) {
            const size_t end = path.find_first_of(".[");
            return path.substr(0, end);
        }

        void validate_project_manifest(const Environment& manifest) {
            const std::set<std::string> collections = {"units", "sources", "code"};
            const std::map<std::string, std::set<std::string>> fields = {
                {"units", {"name", "unit"}},
                {"sources", {"name", "filepath"}},
                {"code", {"file", "string"}},
            };

            for (const auto& [path, collection] : manifest.hierarchy.get_collections()) {
                (void)collection;
                if (collections.find(top_level_name(path)) == collections.end()) {
                    throw SyntaxException(
                        "Invalid DIP project manifest",
                        "The manifest contains the unsupported top-level collection `" + top_level_name(path) + "`.",
                        "Use only the units[], sources[], and code[] collections.",
                        __FILE__,
                        __LINE__
                    );
                }
            }

            for (size_t index = 0; index < manifest.nodes.size(); ++index) {
                const auto& node = manifest.nodes.at(index);
                const std::string& path = node->path.name;
                const std::string top = top_level_name(path);
                const size_t item_end = path.find(']');
                const size_t field_start = (item_end == std::string::npos) ? std::string::npos : item_end + 2;
                const bool valid_item = collections.find(top) != collections.end() && item_end != std::string::npos &&
                                        field_start != std::string::npos && field_start < path.size() &&
                                        path.find('.', field_start) == std::string::npos &&
                                        fields.at(top).find(path.substr(field_start)) != fields.at(top).end();
                if (!valid_item) {
                    throw SyntaxException(
                        "Invalid DIP project manifest",
                        "The manifest node `" + path + "` is not part of the DIPfile schema.",
                        "Use only the declared fields of units[], sources[], and code[] items.",
                        __FILE__,
                        __LINE__,
                        node->line
                    );
                }
            }
        }

        std::filesystem::path project_path(const std::filesystem::path& base, const std::string& value) {
            const std::filesystem::path path(value);
            return path.is_absolute() ? path : base / path;
        }

    } // namespace

    int DIP::num_instances = 0;

    DIP::DIP() {
        // initial settings
        instance_number = DIP::num_instances++;
        source.name = "DIP" + std::to_string(instance_number);
        source.line_number = 0;
        env.sources.append(source.name, "", "", {"", 0});

        // populate node lists
        nodes_nohierarchy.insert(nodes_nohierarchy.end(), nodes_special.begin(), nodes_special.end());
        nodes_nohierarchy.insert(nodes_nohierarchy.end(), nodes_properties.begin(), nodes_properties.end());
        nodes_notypes.insert(nodes_notypes.end(), nodes_special.begin(), nodes_special.end());
        nodes_notypes.insert(nodes_notypes.end(), nodes_properties.begin(), nodes_properties.end());
        nodes_notypes.insert(nodes_notypes.end(), nodes_hierarchy.begin(), nodes_hierarchy.end());
    }

    DIP::DIP(const Source& src) {
        // initial settings
        instance_number = DIP::num_instances++;
        source = src;
        env.sources.append(source.name, "", "", {"", 0});

        // populate node lists
        nodes_nohierarchy.insert(nodes_nohierarchy.end(), nodes_special.begin(), nodes_special.end());
        nodes_nohierarchy.insert(nodes_nohierarchy.end(), nodes_properties.begin(), nodes_properties.end());
        nodes_notypes.insert(nodes_notypes.end(), nodes_special.begin(), nodes_special.end());
        nodes_notypes.insert(nodes_notypes.end(), nodes_properties.begin(), nodes_properties.end());
        nodes_notypes.insert(nodes_notypes.end(), nodes_hierarchy.begin(), nodes_hierarchy.end());
    }

    void DIP::add_string(const std::string& source_code) {
        add_string_input(source_code, env.sources.at(source.name).path, {source.name, source.line_number});
    }

    void DIP::add_file(const std::filesystem::path& source_file, std::string source_name, bool absolute) {

        add_file_input(source_file, std::move(source_name), absolute, {source.name, source.line_number});
    }

    void DIP::add_string_input(
        const std::string& source_code,
        const std::filesystem::path& source_file,
        const Source& parent,
        std::string source_name
    ) {
        if (source_name.empty()) {
            source_name = source.name + "_" + std::string(STRING_SOURCE) + std::to_string(num_strings);
            num_strings++;
        }
        env.sources.append(source_name, source_file, source_code, parent);
        parse_lines(lines, source_code, source_name);
    }

    void DIP::add_file_input(
        const std::filesystem::path& source_file, std::string source_name, bool absolute, const Source& parent
    ) {

        // prepare source data
        std::ifstream file(source_file);
        if (!file)
            throw dip::IOException(
                "File not found",
                "The file `" + source_file.string() + "` could not be opened.",
                "Check whether the file exists and whether you have sufficient permissions.",
                __FILE__,
                __LINE__
            );
        std::ostringstream source_code;
        source_code << file.rdbuf();
        if (source_name.empty()) {
            // TODO: implement 'absolute' switch
            source_name = source.name + "_" + std::string(FILE_SOURCE) + std::to_string(num_files);
            num_files++;
        }

        // create a new source
        (void)absolute;
        env.sources.append(source_name, source_file, source_code.str(), parent);

        // parse lines from the source code
        parse_lines(lines, source_code.str(), source_name);
    }

    void DIP::add_source(const std::string& sname, const std::string& spath) {
        add_source_input(sname, spath, {source.name, source.line_number});
    }

    void DIP::add_source_input(const std::string& sname, const std::string& spath, const Source& parent) {
        std::string source_name = source.name + "_" + std::string(DIRECT_SOURCE) + std::to_string(num_sources);
        num_sources++;
        Source sparent = {source_name, parent.line_number};
        EnvSource senv = parse_source(sname, spath, sparent);
        senv.parent = parent;
        env.sources.append(sname, senv);
    }

    void DIP::add_unit(const std::string& uname, const std::string& uexpr) {
        env.units.append(uname, uexpr, source.name);
    }

    void DIP::add_project(const std::filesystem::path& project_file) {
        const std::filesystem::path absolute_project = std::filesystem::absolute(project_file);
        const std::string project_source = source.name + "_project" + std::to_string(num_projects++);

        DIP manifest;
        manifest.add_string(std::string(project_preamble));
        manifest.add_file(absolute_project, project_source, true);
        const Environment project = manifest.parse();
        validate_project_manifest(project);

        const EnvSource& project_definition = project.sources.at(project_source);
        env.sources.append(project_source, absolute_project, project_definition.code, {source.name, source.line_number});
        const std::filesystem::path project_directory = absolute_project.parent_path();

        if (project.hierarchy.has_collection("units")) {
            const Collection& collection = project.hierarchy.get_collection("units");
            for (const std::string& index : collection.items) {
                const std::string item = "units[" + index + "]";
                add_unit(project[item + ".name"].as<std::string>(), project[item + ".unit"].as<std::string>());
            }
        }
        if (project.hierarchy.has_collection("sources")) {
            const Collection& collection = project.hierarchy.get_collection("sources");
            for (const std::string& index : collection.items) {
                const std::string item = "sources[" + index + "]";
                const auto filepath = project_path(project_directory, project[item + ".filepath"].as<std::string>());
                const auto node = project.get_node(item + ".filepath");
                add_source_input(
                    project[item + ".name"].as<std::string>(),
                    filepath.string(),
                    {project_source, node->line.source.line_number}
                );
            }
        }
        if (project.hierarchy.has_collection("code")) {
            const Collection& collection = project.hierarchy.get_collection("code");
            for (const std::string& index : collection.items) {
                const std::string item = "code[" + index + "]";
                const auto file = project.get_node(item + ".file");
                const auto string = project.get_node(item + ".string");
                const bool has_file = file->value != nullptr;
                const bool has_string = string->value != nullptr;
                if (has_file == has_string) {
                    throw SyntaxException(
                        "Invalid DIP project code entry",
                        "Each code[] item must define exactly one of `file` or `string`.",
                        "Set one field and leave the other as none.",
                        __FILE__,
                        __LINE__,
                        file->line
                    );
                }
                const auto location_node = has_file ? file : string;
                const Source parent = {project_source, location_node->line.source.line_number};
                if (has_file) {
                    add_file_input(project_path(project_directory, project[item + ".file"].as<std::string>()), {}, true, parent);
                } else {
                    add_string_input(project[item + ".string"].as<std::string>(), absolute_project, parent);
                }
            }
        }
    }

    void DIP::add_function_value(const std::string& name, FunctionList::DataFunctionType func) {
        env.functions.append_value(name, std::move(func), source.name);
    }

    void DIP::add_function_nodes(const std::string& name, FunctionList::NodesFunctionType func) {
        env.functions.append_nodes(name, std::move(func), source.name);
    }

    std::string DIP::to_string() {
        return "DIP";
    }

    // Set nodes that can preceeding an option
    static constexpr std::array<NodeDtype, 5> preceeding_nodes = {
        NodeDtype::Boolean, NodeDtype::Integer, NodeDtype::Float, NodeDtype::String, NodeDtype::Table
    };

    inline void check_indent(const BaseNode::PointerType& previous_node, const BaseNode::PointerType& current_node) {
        // We make sure that the indent spacing is always set by INDENT_STEP
        if ((current_node->indent % INDENT_STEP) != 0) {
            std::stringstream suggested;
            suggested << (current_node->indent - (current_node->indent % INDENT_STEP)) << ", ";
            suggested << (current_node->indent - (current_node->indent % INDENT_STEP) + INDENT_STEP) << ", ...";
            throw dip::SyntaxException(
                "Invalid indent length",
                "The indentation length is " + std::to_string(current_node->indent) + ", which is not a multiple of " +
                    std::to_string(INDENT_STEP) + ".",
                "Use an indentation length of " + suggested.str(),
                __FILE__,
                __LINE__,
                current_node->line
            );
        }
        if (previous_node != nullptr) {
            if ((current_node->indent > previous_node->indent) &&
                (current_node->indent - previous_node->indent) != INDENT_STEP) {
                throw dip::SyntaxException(
                    "Child node has an invalid indent",
                    "The child node is indented " + std::to_string(current_node->indent) +
                        " spaces, but it should be " + std::to_string(previous_node->indent + INDENT_STEP) + " spaces.",
                    "Indent the child node " + std::to_string(INDENT_STEP) + " spaces more than the preceding node.",
                    __FILE__,
                    __LINE__,
                    current_node->line
                );
            }
        }
    }

    inline void set_node_property(const BaseNode::PointerType& current_node, const BaseNode::PointerType& previous_node) {
        // assign properties to the previous value node
        PropertyNode::PointerType pnode = std::dynamic_pointer_cast<PropertyNode>(current_node);
        if (!previous_node || std::find(preceeding_nodes.begin(), preceeding_nodes.end(), previous_node->dtype) ==
                                  preceeding_nodes.end()) {
            std::stringstream ss;
            for (size_t i = 0; i < preceeding_nodes.size(); i++) {
                if (i == preceeding_nodes.size() - 1)
                    ss << " and ";
                else if (i > 0)
                    ss << ", ";
                ss << NodeDtypeNames.at(preceeding_nodes[i]);
            }
            throw dip::SyntaxException(
                "Cannot set a property on a non-value node",
                "Only value nodes (" + ss.str() + ") can have properties, but the node type is `" +
                    NodeDtypeNames.at(previous_node->dtype) + "`.",
                "Remove the property or move it behind a value node.",
                __FILE__,
                __LINE__,
                pnode->line
            );
        }
        if (previous_node->indent >= pnode->indent || (pnode->indent - previous_node->indent) != INDENT_STEP)
            throw dip::SyntaxException(
                "Node property has an invalid indent",
                "The property is indented " + std::to_string(pnode->indent) + " spaces, but it should be " +
                    std::to_string(previous_node->indent + INDENT_STEP) + " spaces.",
                "Indent the property " + std::to_string(INDENT_STEP) + " spaces more than the preceding node.",
                __FILE__,
                __LINE__,
                pnode->line
            );
        if (!previous_node->set_property(pnode->ptype, pnode->value_raw, pnode->units_raw))
            throw dip::SyntaxException(
                "Node property could not be set",
                "The property could not be assigned to the preceding value node `" + previous_node->path.name + "`.",
                "Check the property and the preceding node.",
                __FILE__,
                __LINE__,
                pnode->line
            );
    }

    Environment DIP::parse() {
        NodeList<BaseNode> queue = parse_code_nodes(lines);
        NodeList<BaseNode> queue_filtered;

        // set properties to nodes and aggregate schemas
        BaseNode::PointerType previous_node = nullptr;
        for (size_t i = 0; i < queue.size(); ++i) {
            BaseNode::PointerType current_node = queue.at(i);
            if (current_node->dtype == NodeDtype::Property) {
                // set property to a node
                set_node_property(current_node, previous_node);
            } else if (current_node->dtype == NodeDtype::Schema) {
                // create a schema and aggregate all child nodes
                check_indent(previous_node, current_node);
                BaseNode::ListType schema_nodes;
                while (i + 1 < queue.size()) {
                    BaseNode::PointerType schema_node = queue.at(i + 1);
                    check_indent(previous_node, schema_node);
                    if (schema_node->indent <= current_node->indent)
                        break;
                    schema_node->indent -= current_node->indent; // strip schema indent from aggregated nodest
                    if (schema_node->dtype == NodeDtype::Property) {
                        set_node_property(schema_node, previous_node);
                    } else {
                        schema_nodes.push_back(schema_node);
                        previous_node = schema_node;
                    }
                    i++;
                }
                if (schema_nodes.size() > 0) {
                    env.schemas.append(current_node->value_raw.at(0), schema_nodes, current_node->line.source.name);
                } else {
                    throw dip::SyntaxException(
                        "Schema does not contain any value nodes",
                        "The schema is empty and does not contain any value nodes.",
                        "Declare or define value nodes in the schema.",
                        __FILE__,
                        __LINE__,
                        current_node->line
                    );
                }
            } else {
                // push rest of the nodes to the filtered node queue
                if (previous_node)
                    check_indent(previous_node, current_node);
                if (current_node->dtype != NodeDtype::Empty)
                    previous_node = current_node;
                queue_filtered.push_back(current_node);
            }
        }
        queue = std::move(queue_filtered);

        // parse other nodes
        Environment target = env;
        while (queue.size() > 0) {
            BaseNode::PointerType node = queue.pop_front();
            // Perform specific node parsing outside of a condition block or inside of a valid condition block
            if (!target.branching.false_case() || node->dtype == NodeDtype::Case) {
                BaseNode::ListType parsed = node->parse(target);
                if (parsed.size() > 0) {
                    while (parsed.size() > 0) {
                        queue.push_front(parsed.back());
                        parsed.pop_back();
                    }
                    continue;
                }
            }
            // Create hierarchical names
            target.hierarchy.record(node, nodes_nohierarchy);
            // Add nodes to the node list
            if (std::find(nodes_notypes.begin(), nodes_notypes.end(), node->dtype) != nodes_notypes.end()) {
                continue;
            } else if (node->dtype == NodeDtype::Case) {
                target.branching.solve_case(node);
            } else if (target.branching.false_case()) {
                continue;
            } else {
                target.branching.prepare_node(node);
                // Clean node name from cases
                node->path = Path(target.branching.clean_name(node->path.name));
                //  If node was previously defined, modify its value
                bool new_node = true;
                for (size_t i = 0; i < target.nodes.size(); i++) {
                    if (target.nodes.at(i)->path.name == node->path.name) {
                        ValueNode::PointerType mnode = target.nodes.at(i);
                        mnode->validate_constant();
                        mnode->modify_value(node, target);
                        new_node = false;
                    }
                }
                if (new_node) {
                    if (node->dtype == NodeDtype::Modification) {
                        std::string prefix = source.name + "_" + std::string(STRING_SOURCE);
                        if (node->line.source.name.compare(0, prefix.size(), prefix) == 0)
                            throw dip::SyntaxException(
                                "Modifying undefined node",
                                "The node type has not been defined and no previous node definition was found.",
                                "Specify a type such as `bool` or `float`, or add a declaration before this node.",
                                __FILE__,
                                __LINE__,
                                node->line
                            );
                    }
                    ValueNode::PointerType vnode = std::dynamic_pointer_cast<ValueNode>(node);
                    if (!vnode)
                        throw dip::ParserException(
                            "Cannot insert a node without a value into an environment",
                            "The node value is undefined and must be set before insertion into the environment.",
                            "Check why the node value was not set correctly.",
                            __FILE__,
                            __LINE__,
                            vnode->line
                        );
                    target.nodes.push_back(vnode);
                }
            }
        }
        // Validate nodes
        for (size_t i = 0; i < target.nodes.size(); i++) {
            ValueNode::PointerType vnode = target.nodes.at(i);
            if (vnode) {
                if (!vnode->schemas.empty())
                    throw dip::SyntaxException(
                        "Invalid value-node schema",
                        "The value node `" + vnode->path.name + "` has applied schemas.",
                        "Apply schemas to a group, map item, or list item rather than to a value node.",
                        __FILE__,
                        __LINE__,
                        vnode->line
                    );
                vnode->validate_definition();
                vnode->validate_options();
                vnode->validate_condition(target);
                vnode->validate_format();
            } else {
                throw dip::ParserException(
                    "Node in a node list has undefined value",
                    "The node value is undefined and must be set before adding it to the node list.",
                    "Check why the node value was not set correctly.",
                    __FILE__,
                    __LINE__,
                    target.nodes.at(i)->line
                );
            }
        }
        return target;
    }

} // namespace snt::dip
