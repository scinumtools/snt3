#include "exs/exs.h"

#include <vector>

enum CustomOperatorType {
  ARRAY_OPERATOR = exs::NUM_OPERATOR_TYPES
};

typedef std::vector<double> AtomValueType;

class CustomAtom : public exs::AtomBase<AtomValueType> {
public:
  CustomAtom(CustomAtom& a) : AtomBase(a) {};
  CustomAtom(AtomValueType v) : AtomBase(v) {};
  static AtomValueType from_string(std::string s) {
    return {std::stod(s)};
  }
  std::string to_string();
  void math_add(CustomAtom* other);
  void math_subtract(CustomAtom* other);
  void math_negate();
};

class OperatorArray : public exs::OperatorGroup<CustomAtom> {
public:
  OperatorArray() : OperatorGroup<CustomAtom>("arr", {"", "[", "]", ","}, ARRAY_OPERATOR) {};
  void operate_group(exs::TokenListBase<CustomAtom>* tokens);
};
