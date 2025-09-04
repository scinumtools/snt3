#ifndef DIP_NODE_BASE_H
#define DIP_NODE_BASE_H

#include "../../val.h"
#include "node.h"
#include "parser.h"

#include <deque>

namespace snt::dip {

  class Environment;

  class BaseNode : virtual public Node {
  public:
    NodeDtype dtype; // data type of a node; in Python this was 'keyword' variable in Node class
    size_t branch_id;
    size_t case_id;
    typedef std::shared_ptr<BaseNode> PointerType;
    typedef std::deque<BaseNode::PointerType> NodeListType;
    BaseNode() : dtype(NodeDtype::None) {};
    BaseNode(const NodeDtype dt) : dtype(dt) {};
    BaseNode(Parser& parser, const NodeDtype dt);
    virtual ~BaseNode() = default;
    virtual NodeListType parse(Environment& env);
    virtual bool set_property(PropertyType property, val::Array::StringType& values, std::string& units);
  };

} // namespace snt::dip

#endif // DIP_NODE_BASE_H
