#import "main.h"

std::string CustomAtom::to_string() {
  if (std::holds_alternative<std::string>(value)) {
    return std::get<std::string>(value);
  } else if (std::holds_alternative<size_t>(value)) {
    return std::to_string(std::get<size_t>(value));
  } else {
    if (std::get<bool>(value) == 0)
      return "false";
    else
      return "true";
  }
}

void CustomAtom::comparison_less(CustomAtom* other) {
  size_t value_this;
  size_t value_other;
  if (std::holds_alternative<std::string>(value)) {
    value_this = std::get<std::string>(value).size();
  } else {
    value_this = std::get<size_t>(value);
  }
  if (std::holds_alternative<std::string>(other->value)) {
    value_other = std::get<std::string>(other->value).size();
  } else {
    value_other = std::get<size_t>(other->value);
  }
  value = value_this < value_other;
}

void CustomAtom::custom_length() {
  value = std::get<std::string>(value).size();
}
