#include "snt/exs.h"

#include <vector>

using namespace snt;

enum CustomOperatorType {
  ARRAY_OPERATOR = exs::NUM_OPERATOR_TYPES
};

typedef std::vector<double> AtomValueType;

class CustomAtom final: public exs::AtomBase<CustomAtom,AtomValueType> {
public:
  CustomAtom(CustomAtom& a) : AtomBase(a) {};
  CustomAtom(AtomValueType v) : AtomBase(v) {};
  static AtomValueType from_string(std::string s, exs::BaseSettings* set = nullptr) {
    return {std::stod(s)};
  }
  std::string to_string() override;
  void math_add(CustomAtom* other) override;
  void math_subtract(CustomAtom* other) override;
  void math_negate() override;
};

class OperatorArray : public exs::OperatorGroup<0> {
public:
  OperatorArray() : OperatorGroup<0>("arr", {"", "[", "]", ","}, ARRAY_OPERATOR) {};
  void operate_group(exs::TokenListBase* tokens);
};
