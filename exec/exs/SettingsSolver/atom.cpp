#import "main.h"

int CustomAtom::from_string(std::string& s, exs::BaseSettings* set) {
  Settings* cset = static_cast<Settings*>(set);
  if (s == cset->symbol)
    return cset->value;
  else
    return std::stoi(s);
}

std::string CustomAtom::to_string() {
  return std::to_string(value);
}

void CustomAtom::math_add(CustomAtom* other) {
  value = value + other->value;
}

void CustomAtom::math_subtract(CustomAtom* other) {
  value = value - other->value;
}

void CustomAtom::math_negate() {
  value = -value;
}
