#include <sstream>

#include "main.h"

std::string CustomAtom::to_string() {
  std::stringstream ss;
  ss << "{";
  for (auto item : value) {
    ss << item << " ";
  }
  ss << std::scientific;
  std::string s = ss.str();
  return s.substr(0, s.size() - 1) + "}";
}

void CustomAtom::math_add(CustomAtom* other) {
  if (value.size() != other->value.size()) {
    throw std::logic_error("Arrays have different sizes: " + std::to_string(value.size()) + " != " + std::to_string(other->value.size()));
  }
  AtomValueType nv;
  for (size_t i = 0; i < value.size(); i++) {
    nv.push_back(value[i] + other->value[i]);
  }
  value = nv;
}

void CustomAtom::math_subtract(CustomAtom* other) {
  if (value.size() != other->value.size()) {
    throw std::logic_error("Arrays have different sizes: " + std::to_string(value.size()) + " != " + std::to_string(other->value.size()));
  }
  AtomValueType nv;
  for (size_t i = 0; i < value.size(); i++) {
    nv.push_back(value[i] - other->value[i]);
  }
  value = nv;
}

void CustomAtom::math_negate() {
  AtomValueType nv;
  for (size_t i = 0; i < value.size(); i++) {
    nv.push_back(-value[i]);
  }
  value = nv;
}
