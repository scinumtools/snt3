#include <snt/val/to_string.h>
#include <string>

namespace snt::val {

  std::string to_string(val::BaseValue::PointerType value, int precision) {
    snt::StringFormatType fmt;
    fmt.valuePrecision = precision;
    return value->to_string(fmt);
  }

  std::string to_string(const val::Array::ShapeType& shape) {
    std::stringstream ss;
    ss << "[";
    for (int i = 0; i < shape.size(); i++) {
      if (i > 0)
        ss << ",";
      ss << std::to_string(shape[i]);
    }
    ss << "]";
    return ss.str();
  }

} // namespace snt::val
