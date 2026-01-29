#include "snt/exs.h"

#include <array>

using namespace snt;

struct Settings: exs::EmptySettings {
  std::string symbol;
  int value;
  std::array<int, 5> options;

  Settings(std::string s, int v, std::array<int, 5> o) 
    : symbol(s), value(v), options(o) {}
};

enum CustomOperatorType {
  SELECT_OPERATOR = exs::NUM_OPERATOR_TYPES
};

class CustomAtom final : public exs::AtomBase<CustomAtom,int> {
public:
  CustomAtom(CustomAtom& a) : AtomBase(a) {};
  CustomAtom(int v) : AtomBase(v) {};
  static int from_string(std::string& s, exs::EmptySettings* set = nullptr);
  std::string to_string() override;
  void math_add(CustomAtom* other) override;
  void math_subtract(CustomAtom* other) override;
  void math_negate() override;
};

class OperatorSelect : public exs::OperatorGroup<1, Settings> {
public:
  OperatorSelect();
  void operate_group(exs::TokenListBase* tokens, Settings* settings);
};
