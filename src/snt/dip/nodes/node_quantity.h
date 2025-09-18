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
    QuantityNode(puq::Quantity::PointerType unt = nullptr): units(std::move(unt)) {};
    virtual ~QuantityNode() = default;
    std::string to_string(const snt::StringFormatType& format = snt::StringFormatType()) const override;
  };

} // namespace snt::dip

#endif // DIP_NODE_QUANTITY_H
