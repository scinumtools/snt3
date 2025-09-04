#ifndef DIP_NODE_H
#define DIP_NODE_H

#include "../settings.h"
#include "../../val.h"

namespace snt::dip {

  class Node {
  public:
    Line line;                            // source code line information; in Python this were 'code' & 'source' variables
    size_t indent;                        // indent of a node
    std::string name;                     // node name
    std::array<std::string, 3> dtype_raw; // data type properties (unsigned/type/precision)
    val::Array::StringType value_raw;     // raw value string(s)
    val::Array::ShapeType value_shape;    // shape of an array value
    ValueOrigin value_origin;             // origin of the value; in Python there were separate variables:
                                          // value_ref, value_expr, value_func
    val::Array::RangeType value_slice;    // slice of an injected node value
    std::string units_raw;                // raw units string
    val::Array::RangeType dimension;      // list of array dimensions
    Node() : indent(0), value_origin(ValueOrigin::String) {};
    Node(const Line& l) : line(l), indent(0), value_origin(ValueOrigin::String) {};
    Node(const std::string& nm) : name(nm), indent(0), value_origin(ValueOrigin::String) {};
    virtual ~Node() = default;
    std::string to_string();
  };

} // namespace snt::dip

#endif // DIP_NODE_H
