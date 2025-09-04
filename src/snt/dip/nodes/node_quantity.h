#ifndef DIP_NODE_QUANTITY_H
#define DIP_NODE_QUANTITY_H

#include "../../puq.h"
#include "node_value.h"

namespace snt::dip {

  class QuantityNode : virtual public ValueNode {
  public:
    typedef std::shared_ptr<QuantityNode> PointerType;
    puq::Quantity::PointerType units;
    void set_units(puq::Quantity::PointerType units_input = nullptr);
    virtual ~QuantityNode() = default;
  };

} // namespace snt::dip

#endif // DIP_NODE_QUANTITY_H
