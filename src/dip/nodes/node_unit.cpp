#include "../dip.h"
#include "../environment.h"
#include "../parsers.h"
#include "nodes.h"

#include <array>
#include <fstream>

namespace dip {

  BaseNode::PointerType UnitNode::is_node(Parser& parser) {
    if (parser.kwd_unit()) {
      // TODO: implement import of a unit
      // TODO: implement injection of a unit
      parser.part_keyword();
      parser.part_equal();
      parser.part_string();
      parser.part_comment();
      return std::make_shared<UnitNode>(parser);
    }
    return nullptr;
  }

  BaseNode::NodeListType UnitNode::parse(Environment& env) {
    EnvUnit senv = {value_raw.at(0), value_raw.at(1)};
    env.units.append(value_raw.at(0), senv);
    return {};
  }

} // namespace dip
