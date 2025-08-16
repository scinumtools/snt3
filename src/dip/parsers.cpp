#include <fstream>
#include <iostream>

#include "dip.h"
#include "helpers.h"
#include "parsers.h"

namespace dip {

  EnvSource parse_source(const std::string& source_name, const std::string& source_file,
                         const Source& parent) {
    if (source_file.size() >= FILE_SUFFIX_DIP.size() &&
        source_file.compare(source_file.size() - FILE_SUFFIX_DIP.size(), FILE_SUFFIX_DIP.size(),
                            FILE_SUFFIX_DIP) == 0) {
      DIP d(parent);
      d.add_file(source_file, source_name);
      Environment senv = d.parse();
      return EnvSource(
          {source_name, source_file, senv.sources.at(source_name).code, parent, senv.nodes});
    } else {
      std::ifstream file(source_file);
      if (!file)
        throw std::runtime_error("Following file could not be found: " + source_file);
      std::ostringstream source_code;
      source_code << file.rdbuf();
      return EnvSource({source_name, source_file, source_code.str(), parent});
    }
  }

  std::queue<Line> parse_lines(std::queue<Line>& lines, const std::string& source_code,
                               const std::string& source_name) {
    // parse nodes from a text
    std::string line;
    std::istringstream ss(source_code);
    int line_number = 0;
    while (std::getline(ss, line)) {
      // skip empty lines
      if (!line.empty() and !std::all_of(line.begin(), line.end(), isspace))
        lines.push({line, {source_name, line_number}});
      line_number++;
    }
    return lines;
  }

  BaseNode::NodeListType parse_code_nodes(std::queue<Line>& lines) {
    BaseNode::NodeListType nodes;
    while (!lines.empty()) {
      Line line = lines.front();
      lines.pop();

      // group block strings
      size_t pos = 0;
      if ((pos = line.code.find(SIGN_BLOCK)) != std::string::npos) { // opening block quotes
        pos += SIGN_BLOCK.length();
        std::ostringstream oss;
        oss << line.code;
        if (line.code.find(SIGN_BLOCK, pos) ==
            std::string::npos) { // closing block quotes on the same line
          while (lines.size() > 0) {
            Line block_line = lines.front();
            lines.pop();
            oss << SEPARATOR_NEWLINE << block_line.code;
            if (block_line.code.find(SIGN_BLOCK) !=
                std::string::npos) { // closing block quotes on a subsequent line
              break;
            }
          }
        }
        line.code = oss.str();
      }

      // add replacement mark for escape symbols
      Parser::encode_escape_symbols(line.code);

      // determine node type
      Parser parser(line);
      BaseNode::PointerType node = nullptr;
      node = EmptyNode::is_node(parser);
      if (node == nullptr)
        parser.part_indent();
      if (node == nullptr)
        node = ImportNode::is_node(parser);
      if (node == nullptr)
        node = UnitNode::is_node(parser);
      if (node == nullptr)
        node = SourceNode::is_node(parser);
      if (node == nullptr)
        node = CaseNode::is_node(parser);
      if (node == nullptr)
        node = PropertyNode::is_node(parser);
      if (node == nullptr)
        parser.part_name();
      if (node == nullptr)
        node = GroupNode::is_node(parser);
      if (node == nullptr)
        node = ImportNode::is_node(parser);
      if (node == nullptr)
        node = ModificationNode::is_node(parser);
      if (node == nullptr)
        parser.part_type();
      if (node == nullptr)
        node = BooleanNode::is_node(parser);
      if (node == nullptr)
        node = IntegerNode::is_node(parser);
      if (node == nullptr)
        node = FloatNode::is_node(parser);
      if (node == nullptr)
        node = StringNode::is_node(parser);
      if (node == nullptr)
        node = TableNode::is_node(parser);

      // make sure that everything was parsed
      if (node == nullptr)
        throw std::runtime_error("Node could not be determined from : " + line.code);
      if (parser.do_continue())
        throw std::runtime_error("Could not parse all text on the line: " + line.code);

      // convert escape symbols to original characterss
      for (auto & rval : node->value_raw)
        Parser::decode_escape_symbols(rval);
      Parser::decode_escape_symbols(node->line.code);

      // put node into the list
      nodes.push_back(node);
    }
    return nodes;
  }

  BaseNode::NodeListType parse_table_nodes(std::queue<Line>& lines, const char delimiter) {
    // parse nodes from a table header
    BaseNode::NodeListType nodes;
    while (!lines.empty()) {
      Line line = lines.front();
      lines.pop();
      trim(line.code);
      // stop when the end of the table header is reached
      if (line.code == SEPARATOR_TABLE_HEADER)
        break;
      // parse a node from the current line
      Parser parser(line);
      parser.part_name();
      parser.part_space();
      parser.part_type();
      parser.part_dimension();
      parser.part_units();
      if (parser.do_continue())
        throw std::runtime_error("Incorrect header format: " + line.code);
      // initialize actual node
      BaseNode::PointerType node(nullptr);
      if (node == nullptr)
        node = BooleanNode::is_node(parser);
      if (node == nullptr)
        node = IntegerNode::is_node(parser);
      if (node == nullptr)
        node = FloatNode::is_node(parser);
      if (node == nullptr)
        node = StringNode::is_node(parser);
      // make sure that everything was parsed
      if (node == nullptr)
        throw std::runtime_error("Node could not be determined from : " + line.code);
      if (parser.do_continue())
        throw std::runtime_error("Could not parse all text on the line: " + line.code);
      node->value_raw.reserve(lines.size()); // roughly reserve some memory to avoid reallocations
      nodes.push_back(node);
    }
    // read values from the rest of the table data and assign them to the node raw values
    while (!lines.empty()) {
      Line line = lines.front();
      lines.pop();
      trim(line.code);
      // split a table line into node values
      Parser parser(line);
      for (size_t i = 0; i < nodes.size(); i++) {
        // parse delimiter
        if (i > 0) {
          const auto& node = nodes.at(i - 1);
          if (node->value_raw.back().back() == delimiter) {
            // delimiter was parsed with the first value (e.g. if value was not given in
            // quote marks)
            node->value_raw.back().pop_back();
            parser.part_indent();
          } else {
            // delimiter still needs to be parsed
            parser.part_delimiter(delimiter);
          }
        }
        // parse a column value
        const auto& node = nodes.at(i);
        parser.value_raw.clear();
        if (parser.part_string()) {
          node->value_raw.push_back(parser.value_raw.at(0));
        } else {
          throw std::runtime_error("Could not parse column '" + node->name +
                                   "' from the table row: " + line.code);
        }
      }
      if (parser.do_continue())
        throw std::runtime_error("Could not parse all text on the line: " + line.code);
    }
    return nodes;
  }

  std::string parse_array(const std::string& value_string, val::Array::StringType& value_raw,
                          val::Array::ShapeType& value_shape) {
    std::stringstream ss(value_string), rm;
    char ch;

    // test for an openning bracket
    ss.get(ch);
    rm << ch;
    if (ch != SIGN_ARRAY_OPEN)
      throw std::runtime_error("Given source code is not a valid array: " + value_string);

    std::string value;
    bool inString = false;
    int dim = 1;
    value_shape.push_back(0);

    while (ss.get(ch) and dim > 0) {
      rm << ch;
      if (ch == SIGN_ARRAY_OPEN) {
        dim++;
        if (value_shape.size() < dim)
          value_shape.push_back(0);
        value.clear();
      } else if (ch == SEPARATOR_ARRAY) {
        if (!value.empty()) {
          value_raw.push_back(value);
          value.clear();
        }
        value_shape[dim - 1]++;
      } else if (ch == SIGN_ARRAY_CLOSE) {
        if (!value.empty()) {
          value_raw.push_back(value);
          value.clear();
        }
        value_shape[dim - 1]++;
        dim--;
      } else if (ch == '"') {
        value.clear();
        while (ss.get(ch) && ch != '"') {
          value += ch;
          rm << ch;
        }
        rm << ch;
      } else if (ch == '\'') {
        value.clear();
        while (ss.get(ch) && ch != '\'') {
          value += ch;
          rm << ch;
        }
        rm << ch;
      } else if (ch == ' ') {
        continue;
      } else {
        value += ch;
      }
    }

    // Check if all nested arrays are closed
    if (dim != 0)
      throw std::runtime_error("Definition of an array has some unclosed brackets or quotes: " +
                               value_string);

    // Normalize shape and check coherence of nested arrays
    int coef = 1;
    for (int d = 1; d < value_shape.size(); d++) {
      coef *= value_shape[d - 1];
      if (value_shape[d] % coef != 0)
        throw std::runtime_error("Items in dimension " + std::to_string(d + 1) +
                                 " do not have the same shape: " + value_string);
      value_shape[d] /= coef;
    }
    // std::cout << ss.str() << std::endl;
    // std::cout << rm.str() << std::endl;
    return rm.str();
  }

  void parse_value(std::string value_string, val::Array::StringType& value_raw,
                   val::Array::ShapeType& value_shape) {
    trim(value_string);
    value_string.erase(std::remove(value_string.begin(), value_string.end(), '\n'),
                       value_string.end());
    if (value_string.empty())
      throw std::runtime_error("Source code of the value is empty");
    else if (value_string.at(0) == SIGN_ARRAY_OPEN)
      parse_array(value_string, value_raw, value_shape);
    else
      value_raw.push_back(value_string);
  }

  void parse_slices(std::string& value_string, val::Array::RangeType& dimension) {
    std::istringstream ss_dims(value_string);
    std::string dim;
    while (getline(ss_dims, dim, SEPARATOR_DIMENSION)) {
      val::Array::RangeStruct range;
      std::size_t pos = dim.find_first_of(SEPARATOR_SLICE);
      if (pos == std::string::npos) {
	range = { static_cast<size_t>(std::stoul(dim)),
		  static_cast<size_t>(std::stoul(dim)) };
      } else {
        std::string dmin = dim.substr(0, pos), dmax = dim.substr(pos + 1);
        if (dmin == "" and dmax == "")
          range = {0, val::Array::max_range};
        else if (dmin == "")
	  range = { 0, static_cast<size_t>(std::stoul(dmax)) };
        else if (dmax == "")
	  range = { static_cast<size_t>(std::stoul(dmin)), val::Array::max_range };
        else
	  range = { static_cast<size_t>(std::stoul(dmin)),
		    static_cast<size_t>(std::stoul(dmax)) };
      }
      if (range.dmax != val::Array::max_range and range.dmax < range.dmin)
        throw std::runtime_error("Maximum range must be higher or equal than minimum range: " +
                                 dim);
      dimension.push_back(range);
    }
  }

} // namespace dip
