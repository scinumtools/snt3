#ifndef EXS_OPERATOR_EXPONENTIAL_H
#define EXS_OPERATOR_EXPONENTIAL_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

  // exponent
  
  class OperatorExponent : public OperatorGroup<1> {
  public:
    
    OperatorExponent(const OperatorGroupSybols& s = {"exp", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

  // natural logarithm

  class OperatorLogarithm : public OperatorGroup<1> {
  public:
    
    OperatorLogarithm(const OperatorGroupSybols& s = {"log", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

  // logarithm with base 10

  class OperatorLogarithm10 : public OperatorGroup<1> {
  public:
    
    OperatorLogarithm10(const OperatorGroupSybols& s = {"log10", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

  // logarithm with an arbitrary base

  class OperatorLogarithmBase : public OperatorGroup<2> {
  public:
    
    OperatorLogarithmBase(const OperatorGroupSybols& s = {"logb", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

  // square root

  class OperatorSquareRoot : public OperatorGroup<1> {
  public:
    
    OperatorSquareRoot(const OperatorGroupSybols& s = {"sqrt", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

  // cubic root

  class OperatorCubicRoot : public OperatorGroup<1> {
  public:
    OperatorCubicRoot(const OperatorGroupSybols& s = {"cbrt", "(", ")", ","});
    void operate_group(TokenListBase* tokens) override;
  };

  // power function

  class OperatorPower : public OperatorBase {
  public:
    
    OperatorPower(std::string s = "**");
    
    void operate_binary(TokenListBase* tokens) override;
  };

  // power function with an arbitrary base

  class OperatorPowerBase : public OperatorGroup<2> {
  public:
    
    OperatorPowerBase(const OperatorGroupSybols& s = {"powb", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };
    
} // namespace snt::exs

#endif // EXS_OPERATOR_EXPONENTIAL_H
