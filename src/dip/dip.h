#ifndef DIP_DIP_H
#define DIP_DIP_H

#include "environment.h"

#include <array>
#include <iostream>
#include <queue>

namespace dip {

  class DIP {
  private:
    static int num_instances;
    int instance_number;
    Environment env;
    std::queue<Line> lines;

    Source source;

    static constexpr std::array<NodeDtype, 3> nodes_special = {NodeDtype::Empty, NodeDtype::Unit,
                                                               NodeDtype::Source};
    static constexpr std::array<NodeDtype, 6> nodes_properties = {NodeDtype::Property};
    static constexpr std::array<NodeDtype, 1> nodes_hierarchy = {NodeDtype::Group};
    std::vector<NodeDtype> nodes_nohierarchy;
    std::vector<NodeDtype> nodes_notypes;

    size_t num_strings = 0; // counter of code inputs from a string
    size_t num_files = 0;   // counter of code inputs from a file
    size_t num_sources = 0; // number of explicitely added sources
    size_t num_units = 0;   // number of explicitely added units

  public:
    DIP();
    DIP(const Source& src);
    void add_string(const std::string& source_code);
    void add_file(const std::string& source_file, std::string source_name = "",
                  const bool absolute = true);
    void add_source(const std::string& source_name, const std::string& source_file);
    void add_unit(const std::string& name, const double value, const std::string& unit = "");
    void add_value_function(const std::string& name, FunctionList::ValueFunctionType func);
    void add_node_function(const std::string& name, FunctionList::TableFunctionType func);
    Environment parse();
    Environment parse_docs();
    std::string to_string();
  };

} // namespace dip

#endif // DIP_DIP_H
