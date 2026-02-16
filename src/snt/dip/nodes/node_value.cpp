#include <snt/dip/nodes/node_value.h>

#include <sstream>

namespace snt::dip {

  ValueNode::ValueNode(const std::string& nm, const val::DataType vdt)
      : constant(false), value_dtype(vdt) {
    name = nm;
  };

  ValueNode::ValueNode(const std::string& nm, val::BaseValue::PointerType val, puq::Quantity::PointerType unt)
    : constant(false), value_dtype(val->get_dtype()), units(std::move(unt)) {
    name = nm;
    val::Array::ShapeType dims = val->get_shape();
    if (val->get_size() > 1) {
      dimension.clear();
      dimension.reserve(dims.size());
      for (size_t dim : dims)
        dimension.push_back({dim, dim});
    }
    set_value(std::move(val));
  };

  val::BaseValue::PointerType ValueNode::cast_value() {
    return cast_value(value_raw, value_shape);
  }

  val::BaseValue::PointerType ValueNode::cast_value(val::Array::StringType& value_input,
                                                    const val::Array::ShapeType& shape) {
    if (!dimension.empty()) {
      return cast_array_value(value_input, shape);
    } else if (value_input.size() > 1) {
      throw std::runtime_error("Value size is an array but node is defined as scalar: " +
                               line.code);
    } else {
      return cast_scalar_value(value_input.at(0));
    }
  }

  void ValueNode::set_value(val::BaseValue::PointerType value_input) {
    value = nullptr;
    if (value_input == nullptr and !value_raw.empty() and !value_raw.at(0).empty()) {
      value = cast_value();
    } else if (value_input != nullptr) {
      if (value_input->get_dtype() == value_dtype)
        value = std::move(value_input);
      else
        value = value_input->cast_as(value_dtype);
    }
    if (value != nullptr) {
      if (!value_slice.empty()) {
        value = value->slice(value_slice);
      }
      if (dimension.empty()) {
        if (value->get_size() > 1)
          throw std::runtime_error("Assigning array value to the scalar node: " + line.code);
      } else {
        validate_dimensions(); // check if value shape corresponds with dimension ranges
      }
    }
  }

  void ValueNode::set_units(puq::Quantity::PointerType units_input) {
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
  
  void ValueNode::modify_value(const BaseNode::PointerType& node, Environment& env) {
    if (node->dtype != NodeDtype::Modification and node->dtype != dtype)
      throw std::runtime_error("Node '" + name + "' with type '" + dtype_raw.at(1) +
                               "' cannot modify node '" + node->name + "' with type '" +
                               node->dtype_raw.at(1) + "'");
    val::BaseValue::PointerType value = cast_value(node->value_raw, node->value_shape);
    if (!node->units_raw.empty()) {
      if (this->units == nullptr)
        throw std::runtime_error("Trying to convert '" + node->units_raw +
                                 "' into a nondimensional quantity: " + line.code);
      else {
        puq::Quantity quantity(std::move(value), node->units_raw);
        quantity = quantity.convert(*this->units);
        value = std::move(quantity.value.magnitude.value);
      }
    }
    value_raw = node->value_raw;
    set_value(std::move(value));
  }

  bool ValueNode::set_property(PropertyType property, val::Array::StringType& values, std::string& units) {
    switch (property) {
    case PropertyType::Options:
      for (const auto& value_option : values) {
        if (dtype == NodeDtype::Boolean)
          throw std::runtime_error("Option property is not implemented for boolean nodes: " +
                                   line.code);
        // TODO: account for multidimensional arrays as individual options
        val::BaseValue::PointerType ovalue = cast_scalar_value(value_option);
        options.push_back({std::move(ovalue), value_option, units});
      }
      return true;
    case PropertyType::Constant:
      constant = true;
      return true;
    case PropertyType::Tags:
      tags = values;
      return true;
    case PropertyType::Description:
      description += values.at(0);
      return true;
    case PropertyType::Condition:
      condition = values.at(0);
      return true;
    default:
      return false;
    }
  }

  /*
   * Validation of node properties and values
   */

  void ValueNode::validate_constant() const {
    if (constant)
      throw std::runtime_error("Node '" + name +
                               "' is constant and cannot be modified: " + line.code);
  }

  void ValueNode::validate_definition() const {
    if (value == nullptr)
      throw std::runtime_error("Declared node has undefined value: " + line.code);
  }

  void ValueNode::validate_condition() const {
    if (!condition.empty()) {
      if (condition == snt::KEYWORD_FALSE)
        throw std::runtime_error("Node does not satisfy the given condition: " + condition);
      else if (condition == snt::KEYWORD_TRUE)
        return;
      // TODO: implement expression solver
    }
  }

  void ValueNode::validate_options() const {
    if (options.size() > 0) {
      bool match = false;
      for (const auto& option : options) {
        if (option.value and value->compare_equal(option.value.get())->all_of())
          match = true;
      }
      if (!match) {
        std::ostringstream oss;
        for (int i = 0; i < options.size(); i++) {
          if (i > 0)
            oss << ", ";
          oss << options[i].value->to_string();
        }
        throw std::runtime_error("Value " + value->to_string() + " of node '" + name +
                                 "' doesn't match with any option: " + oss.str());
      }
    }
  }

  void ValueNode::validate_format() const {
    if (format.size() > 0)
      throw std::runtime_error("Format property can be used only with string nodes: " +
                               line.code);
  }

  void ValueNode::validate_dimensions() const {
    val::Array::ShapeType vdim = value->get_shape();
    if (dimension.size() != vdim.size())
      throw std::runtime_error("Number of value dimensions does not match that of node: " +
                               std::to_string(vdim.size()) +
                               "!=" + std::to_string(dimension.size()));
    for (size_t i = 0; i < dimension.size(); i++) {
      size_t dmin = dimension[i].dmin;
      size_t dmax = dimension[i].dmax; // dimension ranges can be max(size_t)
      if (dmax == val::Array::max_range)
        dmax = vdim[i];
      if (vdim[i] < dmin or dmax < vdim[i]) {
        std::ostringstream nss, vss;
        for (size_t j = 0; j < dimension.size(); j++) {
          if (j > 0) {
            nss << ",";
            vss << ",";
          }
          dmin = dimension[j].dmin;
          dmax = dimension[j].dmax;
          if (dmin == 0 and dmax == val::Array::max_range)
            nss << SEPARATOR_SLICE;
          else if (dmin == dmax)
            nss << dmin;
          else if (dmax == val::Array::max_range)
            nss << dmin << SEPARATOR_SLICE;
          else if (dmin == 0)
            nss << SEPARATOR_SLICE << dmax;
          else
            nss << dmin << SEPARATOR_SLICE << dmax;
          vss << vdim[j];
        }
        throw std::runtime_error(
            "Value dimensions do not correspond to the node dimension ranges: [" + vss.str() +
            "] != [" + nss.str() + "]");
      }
    }
    // std::cout << "checking dimensions " << std::endl;
    // std::cout << dimension.size() << std::endl;
    // std::cout << value->dimension().size() << std::endl;
  }

} // namespace snt::dip
