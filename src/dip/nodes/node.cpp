#include <sstream>
#include <tuple>

#include "nodes.h"

namespace dip {

  std::string Node::to_string() {
    std::stringstream ss;
    ss << "indent(" << indent << ") ";
    ss << "name(" << name << ") ";
    ss << "value_raw(" << value_raw.at(0) << ") ";
    ss << "dimension(";
    for (auto range : dimension)
      ss << range.dmin << SEPARATOR_SLICE << range.dmax << SEPARATOR_DIMENSION;
    if (!dimension.empty())
      ss.seekp(-1, std::ios_base::end);
    ss << ") ";
    return ss.str();
  }

} // namespace dip
