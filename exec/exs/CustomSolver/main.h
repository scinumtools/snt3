#include "exs.h"

#include <variant>

using namespace snt;

enum CustomOperatorType {
  LENGTH_OPERATOR = exs::NUM_OPERATOR_TYPES
};

typedef std::variant<std::string, bool, size_t> AtomValueType;

class CustomAtom : public exs::AtomBase<AtomValueType> {
public:
  CustomAtom(CustomAtom& a) : AtomBase(a) {};
  CustomAtom(AtomValueType v) : AtomBase(v) {};
  static AtomValueType from_string(std::string s) {
    return s;
  }
  std::string to_string();
  void comparison_less(CustomAtom* other);
  void custom_length();
};

class OperatorLength : public exs::OperatorGroup<CustomAtom, 1> {
public:
  OperatorLength();
  void operate_group(exs::TokenListBase<CustomAtom>* tokens);
};
