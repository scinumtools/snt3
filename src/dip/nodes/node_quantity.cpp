#include "nodes.h"

namespace dip {

  void QuantityNode::set_units(Quantity::PointerType units_input) {
    // setting node units
    units = nullptr;
    if (units_input == nullptr and !units_raw.empty()) {
      units = std::make_unique<puq::Quantity>(units_raw);
    } else if (units_input != nullptr) {
      units = std::move(units_input);
    }
    // converting option units if necessary
    for (size_t i = 0; i < options.size(); i++) {
      std::string option_units = options[i].units_raw;
      if (!option_units.empty()) {
        if (units == nullptr)
          throw std::runtime_error("Trying to convert '" + option_units +
                                   "' into a nondimensional quantity: " + line.code);
        else {
          puq::Quantity quantity(std::move(options[i].value), option_units);
          quantity = quantity.convert(*units);
          options[i].value = std::move(quantity.value.magnitude.value);
        }
      }
    }
  }

} // namespace dip
