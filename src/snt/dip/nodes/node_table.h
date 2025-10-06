#ifndef DIP_NODE_TABLE_H
#define DIP_NODE_TABLE_H

#include "node_base.h"

namespace snt::dip {

  class TableNode : public BaseNode {
  public:
    char delimiter;
    static BaseNode::PointerType is_node(Parser& parser);
    TableNode(Parser& parser)
        : BaseNode(parser, NodeDtype::Table), delimiter(SEPARATOR_TABLE_COLUMNS) {};
    BaseNode::ListType parse(Environment& env) override;
    bool set_property(PropertyType property, val::Array::StringType& values,
                      std::string& units) override;
  };

} // namespace snt::dip

#endif // DIP_NODE_TABLE_H
