#include "solver_template.h"

#include "../nodes/node_value.h"
#include "../nodes/parser.h"

namespace snt::dip {

  TemplateSolver::TemplateSolver(Environment& env) {
    environment = &env;
  }

  std::string TemplateSolver::eval(std::string expression) {
    std::stringstream ss;
    bool openned = false;
    while (!expression.empty()) {
      char c = expression.front(); // get first character
      expression.erase(0, 1);      // remove first character
      if (c == SIGN_REFERENCE_OPEN) {
        openned = true;
        Parser parser({expression, {"TEMPLATE_SOLVER", 0}});
        if (parser.part_reference()) {
          parser.part_slice();
          parser.part_units();
          parser.part_slice();
          parser.part_format();

          // request node from the environment and extract its value
          ValueNode::ListType nodes = environment->request_nodes(parser.value_raw.at(0), RequestType::Reference);
          const ValueNode::PointerType& vnode = nodes.front();

          // apply slicing
          if (!parser.value_slice.empty())
            vnode->value = vnode->value->slice(parser.value_slice);

          // set value string format
          snt::StringFormatType format;
          format.stringQuotes = false;
          if (!parser.formatting[0].empty()) {
            if (parser.formatting[0][0] == '0')
              format.paddingZeros = true;
            format.paddingSize = std::stoi(parser.formatting[0]);
          }
          if (!parser.formatting[2].empty()) {
            format.specifier = parser.formatting[2][0];
          }
          if (!parser.formatting[1].empty()) {
            format.valuePrecision = std::stoi(parser.formatting[1]);
            if (format.specifier == 'g')
              format.valuePrecision += 1;
          }

          // parse value as string
          ss << vnode->to_string(format);
          expression = parser.code;
        }
      } else if (c == SIGN_REFERENCE_CLOSE) {
        if (openned)
          openned = false;
        else
          ss << c;
      } else {
        ss << c;
      }
    }
    return ss.str();
  }

} // namespace snt::dip
