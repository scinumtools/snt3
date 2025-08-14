#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "dip.h"
#include "parsers.h"
#include "settings.h"

namespace dip {

  int DIP::num_instances = 0;

  DIP::DIP() {
    float a;
    // initial settings
    instance_number = DIP::num_instances++;
    source.name = "DIP" + std::to_string(instance_number);
    source.line_number = 0;
    env.sources.append(source.name, "", "", {"", 0});

    // populate node lists
    nodes_nohierarchy.insert(nodes_nohierarchy.end(), nodes_special.begin(), nodes_special.end());
    nodes_nohierarchy.insert(nodes_nohierarchy.end(), nodes_properties.begin(),
                             nodes_properties.end());
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
    nodes_nohierarchy.insert(nodes_nohierarchy.end(), nodes_properties.begin(),
                             nodes_properties.end());
    nodes_notypes.insert(nodes_notypes.end(), nodes_special.begin(), nodes_special.end());
    nodes_notypes.insert(nodes_notypes.end(), nodes_properties.begin(), nodes_properties.end());
    nodes_notypes.insert(nodes_notypes.end(), nodes_hierarchy.begin(), nodes_hierarchy.end());
  }

  void DIP::add_string(const std::string& source_code) {

    // prepare source data
    std::string source_file = env.sources.at(source.name).path;
    std::string source_name =
        source.name + "_" + std::string(STRING_SOURCE) + std::to_string(num_strings);
    num_strings++;

    // create a new source
    env.sources.append(source_name, source_file, source_code, {source.name, source.line_number});

    // parse lines from the source code
    parse_lines(lines, source_code, source_name);
  }

  void DIP::add_file(const std::string& source_file, std::string source_name, const bool absolute) {

    // prepare source data
    std::ifstream file(source_file);
    if (!file)
      throw std::runtime_error("Following file could not be found: " + source_file);
    std::ostringstream source_code;
    source_code << file.rdbuf();
    if (source_name.empty()) {
      // TODO: implement 'absolute' switch
      source_name = source.name + "_" + std::string(FILE_SOURCE) + std::to_string(num_files);
      num_files++;
    }

    // create a new source
    // TODO: treat source lineno and source_file with respect to where this method is called
    env.sources.append(source_name, source_file, source_code.str(),
                       {source.name, source.line_number});

    // parse lines from the source code
    parse_lines(lines, source_code.str(), source_name);
  }

  void DIP::add_source(const std::string& sname, const std::string& spath) {
    std::string source_name =
        source.name + "_" + std::string(DIRECT_SOURCE) + std::to_string(num_sources);
    num_sources++;
    Source sparent = {source_name, 0};
    EnvSource senv = parse_source(sname, spath, sparent);
    env.sources.append(sname, senv);
  }

  void DIP::add_unit(const std::string& name, const double value, const std::string& unit) {
    // TODO: implement
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
  static constexpr std::array<NodeDtype, 13> preceeding_nodes = {
      NodeDtype::Boolean, NodeDtype::Integer, NodeDtype::Float, NodeDtype::String, NodeDtype::Table};

  Environment DIP::parse() {
    NodeList queue = parse_code_nodes(lines);
    // set properties to nodes
    BaseNode::PointerType previous_node = nullptr;
    for (size_t i = 0; i < queue.size(); ++i) {
      BaseNode::PointerType current_node = queue.at(i);
      if (current_node->dtype == NodeDtype::Property) {
        PropertyNode::PointerType pnode = std::dynamic_pointer_cast<PropertyNode>(current_node);
        if (std::find(preceeding_nodes.begin(), preceeding_nodes.end(), previous_node->dtype) ==
            preceeding_nodes.end())
          throw std::runtime_error(
              "Only value nodes (bool, int, float and str) can have properties: " +
              pnode->line.code);
        if (previous_node->indent >= pnode->indent)
          throw std::runtime_error("The indent '" + std::to_string(pnode->indent) +
                                   "' of a property is not higher than the indent '" +
                                   std::to_string(previous_node->indent) +
                                   "' of a preceding node: " + pnode->line.code);
        if (!previous_node->set_property(pnode->ptype, pnode->value_raw, pnode->units_raw))
          throw std::runtime_error("Property could not be set: " + pnode->line.code);
      } else {
        previous_node = current_node;
      }
    }
    // parse other nodes
    Environment target = env;
    while (queue.size() > 0) {
      BaseNode::PointerType node = queue.pop_front();
      if (node->dtype == NodeDtype::Property)
        continue;
      if (!target.branching.false_case() or node->dtype == NodeDtype::Case) {
        // Perform specific node parsing only outside of case or inside of valid case
        BaseNode::NodeListType parsed = node->parse(target);
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
      if (std::find(nodes_notypes.begin(), nodes_notypes.end(), node->dtype) !=
          nodes_notypes.end()) {
        continue;
      } else if (node->dtype == NodeDtype::Case) {
        target.branching.solve_case(node);
      } else if (target.branching.false_case()) {
        continue;
      } else {
        target.branching.prepare_node(node);
        // Clean node name from cases
        node->name = target.branching.clean_name(node->name);
        // Set the node value an unit
        // TODO: maybe this can be done after modifications?!
        ValueNode::PointerType vnode = std::dynamic_pointer_cast<ValueNode>(node);
        if (vnode and vnode->value == nullptr) {
          vnode->set_value();
        }
        QuantityNode::PointerType qnode = std::dynamic_pointer_cast<QuantityNode>(node);
        if (qnode and qnode->units == nullptr) {
          qnode->set_units();
        }
        // If node was previously defined, modify its value
        bool new_node = true;
        for (size_t i = 0; i < target.nodes.size(); i++) {
          if (target.nodes.at(i)->name == node->name) {
            ValueNode::PointerType pnode =
                std::dynamic_pointer_cast<ValueNode>(target.nodes.at(i));
            pnode->validate_constant();
            pnode->modify_value(node, target);
            new_node = false;
          }
        }
        if (new_node) {
          if (node->dtype == NodeDtype::Modification) {
            std::string prefix = source.name + "_" + std::string(STRING_SOURCE);
            if (node->line.source.name.compare(0, prefix.size(), prefix) == 0)
              throw std::runtime_error("Modifying undefined node: " + node->line.code);
          }
          target.nodes.push_back(node);
        }
      }
    }
    // Validate nodes
    for (ssize_t i = 0; i < target.nodes.size(); i++) {
      ValueNode::PointerType vnode = std::dynamic_pointer_cast<ValueNode>(target.nodes.at(i));
      if (vnode) {
        vnode->validate_definition();
        vnode->validate_options();
        vnode->validate_condition();
        vnode->validate_format();
      } else {
        throw std::runtime_error("Detected non-value node in the node list: " +
                                 target.nodes.at(i)->line.code);
      }
    }
    return target;
  }

} // namespace dip
