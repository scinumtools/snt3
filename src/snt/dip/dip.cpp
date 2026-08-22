#include "parsers.h"

#include <fstream>
#include <iostream>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_property.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/settings.h>
#include <sstream>
#include <string>

namespace snt::dip {

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

        // prepare source data
        std::string source_file = env.sources.at(source.name).path;
        std::string source_name = source.name + "_" + std::string(STRING_SOURCE) + std::to_string(num_strings);
        num_strings++;

        // create a new source
        env.sources.append(source_name, source_file, source_code, {source.name, source.line_number});

        // parse lines from the source code
        parse_lines(lines, source_code, source_name);
    }

    void DIP::add_file(const std::filesystem::path& source_file, std::string source_name, bool absolute) {

        // prepare source data
        std::ifstream file(source_file);
        if (!file)
            throw dip::IOException("File not found", source_file.string(), "", "", __FILE__, __LINE__);
        std::ostringstream source_code;
        source_code << file.rdbuf();
        if (source_name.empty()) {
            // TODO: implement 'absolute' switch
            source_name = source.name + "_" + std::string(FILE_SOURCE) + std::to_string(num_files);
            num_files++;
        }

        // create a new source
        // TODO: treat source lineno and source_file with respect to where this method is called
        env.sources.append(source_name, source_file, source_code.str(), {source.name, source.line_number});

        // parse lines from the source code
        parse_lines(lines, source_code.str(), source_name);
    }

    void DIP::add_source(const std::string& sname, const std::string& spath) {
        std::string source_name = source.name + "_" + std::string(DIRECT_SOURCE) + std::to_string(num_sources);
        num_sources++;
        Source sparent = {source_name, 0};
        EnvSource senv = parse_source(sname, spath, sparent);
        env.sources.append(sname, senv);
    }

    void DIP::add_unit(const std::string& uname, const std::string& uexpr) {
        num_units++;
        EnvUnit uenv = {uname, uexpr};
        env.units.append(uname, uenv);
    }

    void DIP::add_value_function(const std::string& name, FunctionList::ValueFunctionType func) {
        env.functions.append_value(name, func);
    }

    void DIP::add_node_function(const std::string& name, FunctionList::TableFunctionType func) {
        env.functions.append_table(name, func);
    }

    std::string DIP::to_string() {
        return "DIP";
    }

    // Set nodes that can preceeding an option
    static constexpr std::array<NodeDtype, 5> preceeding_nodes = {
        NodeDtype::Boolean, NodeDtype::Integer, NodeDtype::Float, NodeDtype::String, NodeDtype::Table
    };

    inline void check_indent(BaseNode::PointerType previous_node, BaseNode::PointerType current_node) {
        // We make sure that the indent spacing is always set by INDENT_STEP
        if ((current_node->indent % INDENT_STEP) != 0) {
            std::stringstream suggested;
            suggested << (current_node->indent - (current_node->indent % INDENT_STEP)) << ", ";
            suggested << (current_node->indent - (current_node->indent % INDENT_STEP) + INDENT_STEP) << ", ...";
            throw dip::SyntaxException(
                "Invalid indent length",
                "Multiple of " + std::to_string(INDENT_STEP) + ".",
                std::to_string(current_node->indent),
                suggested.str(),
                current_node.get(),
                __FILE__,
                __LINE__
            );
        }
        if (previous_node != nullptr) {
            if ((current_node->indent > previous_node->indent) &&
                (current_node->indent - previous_node->indent) != INDENT_STEP) {
                throw dip::SyntaxException(
                    "Child node has an invalid indent",
                    std::to_string(previous_node->indent + INDENT_STEP),
                    std::to_string(current_node->indent),
                    "Indent " + std::to_string(INDENT_STEP) + " spaces more than the preceding node.",
                    current_node.get(),
                    __FILE__,
                    __LINE__
                );
            }
        }
    }

    inline void set_node_property(BaseNode::PointerType current_node, BaseNode::PointerType previous_node) {
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
                "Only value nodes (" + ss.str() + ") can have properties.",
                "Node type is: " + NodeDtypeNames.at(previous_node->dtype),
                "Remove the property or move it behind a value node.",
                pnode.get(),
                __FILE__,
                __LINE__
            );
        }
        if (previous_node->indent >= pnode->indent || (pnode->indent - previous_node->indent) != INDENT_STEP)
            throw dip::SyntaxException(
                "Node property has an invalid indent",
                std::to_string(previous_node->indent + INDENT_STEP),
                std::to_string(pnode->indent),
                "Indent " + std::to_string(INDENT_STEP) + " spaces more than the preceding node.",
                pnode.get(),
                __FILE__,
                __LINE__
            );
        if (!previous_node->set_property(pnode->ptype, pnode->value_raw, pnode->units_raw))
            throw dip::SyntaxException(
                "Node property could not be set",
                "Property is assigned to the preceeding value node.",
                "Property could not be assigned to the preceeing node: " + previous_node->path.name,
                "Check the property and preceeding node.",
                pnode.get(),
                __FILE__,
                __LINE__
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
                    env.schemas.append(current_node->value_raw.at(0), schema_nodes);
                } else {
                    throw dip::SyntaxException(
                        "Schema does not contain any value nodes",
                        "Schema contains some value nodes.",
                        "Schema is empty.",
                        "Declare, or define value nodes in the schema.",
                        current_node.get(),
                        __FILE__,
                        __LINE__
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
                // Set the node value an unit
                // TODO: maybe this can be done after modifications?!
                ValueNode::PointerType vnode = std::dynamic_pointer_cast<ValueNode>(node);
                if (vnode) {
                    if (vnode->value == nullptr)
                        vnode->set_value();
                    if (!vnode->units)
                        vnode->set_units();
                }
                // If node was previously defined, modify its value
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
                                "Node type has to be defined",
                                "Could not find previous node definition",
                                "Specify type (e.g. bool, float, ...) or add declaration prior to this node",
                                node.get(),
                                __FILE__,
                                __LINE__
                            );
                    }
                    if (vnode == nullptr)
                        throw dip::ParserException(
                            "Cannot insert a node without a value into an environment",
                            "Value of a node is set before inserted to the environment.",
                            "Value is undefined",
                            "Check why value was not set properly.",
                            vnode.get(),
                            __FILE__,
                            __LINE__
                        );
                    target.nodes.push_back(vnode);
                }
            }
        }
        // Validate nodes
        for (size_t i = 0; i < target.nodes.size(); i++) {
            ValueNode::PointerType vnode = target.nodes.at(i);
            if (vnode) {
                vnode->validate_definition();
                vnode->validate_options();
                vnode->validate_condition(target);
                vnode->validate_format();
            } else {
                throw dip::ParserException(
                    "Node in a node list has undefined value",
                    "Nodes in a node list have defined values.",
                    "Value is undefined",
                    "Check why value was not set properly.",
                    target.nodes.at(i).get(),
                    __FILE__,
                    __LINE__
                );
            }
        }
        return target;
    }

} // namespace snt::dip
