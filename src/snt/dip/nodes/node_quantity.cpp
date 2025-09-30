#include "node_quantity.h"

namespace snt::dip {

  void QuantityNode::set_units(puq::Quantity::PointerType units_input) {
    // setting node units
    units = nullptr;
    if (units_input == nullptr and !units_raw.empty()) {
      units = std::make_unique<puq::Quantity>(units_raw);
    } else if (units_input != nullptr) {
      units = std::move(units_input);
    }
    // converting option units if necessary
    for (auto& option : options) {
      std::string option_units = option.units_raw;
      if (!option_units.empty()) {
        if (units == nullptr)
          throw std::runtime_error("Trying to convert '" + option_units +
                                   "' into a nondimensional quantity: " + line.code);
        else {
          puq::Quantity quantity(std::move(option.value), option_units);
          quantity = quantity.convert(*units);
          option.value = std::move(quantity.value.magnitude.value);
        }
      }
    }
  }

  std::string QuantityNode::to_string(const snt::StringFormatType& format) const {
    std::stringstream ss;
    ss << value->to_string(format);
    if (units)
      ss << " " << units->to_string();
    return ss.str();
  }

} // namespace snt::dip
