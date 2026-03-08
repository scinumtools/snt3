#include <snt/exs/math/exponential.h>

namespace snt::exs {

  // exponent
  
  OperatorExponent::OperatorExponent(const OperatorGroupSybols& s) :
    OperatorGroup<1>("exp", s, EXPONENT_OPERATOR) {
  }
  
  void OperatorExponent::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_exponent();
    tokens->put_left(group1);
  };

  // logarithm

  OperatorLogarithm::OperatorLogarithm(const OperatorGroupSybols& s) :
    OperatorGroup<1>("log", s, LOGARITHM_OPERATOR) {
  }
  
  void OperatorLogarithm::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_logarithm();
    tokens->put_left(group1);
  };

  // logarithm with a base 10

  OperatorLogarithm10::OperatorLogarithm10(const OperatorGroupSybols& s) :
    OperatorGroup<1>("log10", s, LOGARITHM_10_OPERATOR) {
  }

  void OperatorLogarithm10::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_logarithm_10();
    tokens->put_left(group1);
  };

  // logarithm with an arbitrary base

  OperatorLogarithmBase::OperatorLogarithmBase(const OperatorGroupSybols& s) :
    OperatorGroup<2>("logb", s, LOGARITHM_BASE_OPERATOR) {
  }

  void OperatorLogarithmBase::operate_group(TokenListBase* tokens) {
    Token group2 = tokens->get_left();
    Token group1 = tokens->get_left();
    group1.atom->math_logarithm_base(group2.atom);
    tokens->put_left(group1);
  };

  // square root

  OperatorSquareRoot::OperatorSquareRoot(const OperatorGroupSybols& s) :
    OperatorGroup<1>("sqrt", s, SQUARE_ROOT_OPERATOR) {
  }
  
  void OperatorSquareRoot::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_square_root();
    tokens->put_left(group1);
  };

  // cubic root

  OperatorCubicRoot::OperatorCubicRoot(const OperatorGroupSybols& s) :
    OperatorGroup<1>("cbrt", s, CUBIC_ROOT_OPERATOR) {
  }
  
  void OperatorCubicRoot::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_cubic_root();
    tokens->put_left(group1);
  };
    
  // power
  
  OperatorPower::OperatorPower(std::string s) :
    OperatorBase("pow", s, POWER_OPERATOR) {
  }
  
  void OperatorPower::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->math_power(right.atom);
    tokens->put_left(left);
  };

  // power with an arbitrary base
  
  OperatorPowerBase::OperatorPowerBase(const OperatorGroupSybols& s) :
    OperatorGroup<2>("powb", s, POWER_BASE_OPERATOR) {
  }
  
  void OperatorPowerBase::operate_group(TokenListBase* tokens) {
    Token group2 = tokens->get_left();
    Token group1 = tokens->get_left();
    group1.atom->math_power_base(group2.atom);
    tokens->put_left(group1);
    };

}
