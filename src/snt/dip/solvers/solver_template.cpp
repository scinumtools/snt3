#include "solver_template.h"
#include "../nodes/parser.h"

namespace snt::dip {

  TemplateSolver::TemplateSolver(Environment& env) {
    environment = &env;
  }
  
  std::string TemplateSolver::eval(std::string expression) {
    std::stringstream ss;
    bool openned = false;
    while (!expression.empty()) {
        char c = expression.front();     // get first character
        expression.erase(0, 1);          // remove first character
	if (c==SIGN_REFERENCE_OPEN) {
	  openned = true;
	  Parser parser({expression, {"TEMPLATE_SOLVER", 0}});
	  if (parser.part_reference()) {
	    parser.part_slice();
	    parser.part_units();
	    /*
	    //parser.part_format();
	    */
	    BaseNode::NodeListType nodes = environment->request_nodes(parser.value_raw.at(0), RequestType::Reference);
	    BaseNode::PointerType node = nodes.front();
	    ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
	    ss << vnode->to_string();
	    expression = parser.code;
	  }
	} else if (c==SIGN_REFERENCE_CLOSE) {
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

}
