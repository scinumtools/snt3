#import "main.h"

int CustomAtom::from_string(std::string& s, Settings* set) {
  if (s == set->symbol)
    return set->value;
  else
    return std::stod(s);
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
