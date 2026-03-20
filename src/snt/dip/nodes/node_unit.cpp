#include "node_unit.h"

#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include "../parsers.h"

#include <array>
#include <fstream>

namespace snt::dip {

  BaseNode::PointerType UnitNode::is_node(Parser& parser) {
    if (parser.kwd_unit()) {
      // TODO: implement import of a unit
      // TODO: implement injection of a unit
      parser.part_keyword();
      parser.part_equal();
      parser.part_units('\0');  // an empty space before the units is already taken by ' = ' sign, so we don't enforce it here
      parser.part_comment();
      return std::make_shared<UnitNode>(parser);
    }
    return nullptr;
  }

  BaseNode::ListType UnitNode::parse(Environment& env) {
    EnvUnit senv = {value_raw.at(0), units_raw};
    env.units.append(value_raw.at(0), senv);
    return {};
  }

} // namespace snt::dip
