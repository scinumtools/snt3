#ifndef EXS_OPERATOR_ARITHMETIC_H
#define EXS_OPERATOR_ARITHMETIC_H

#include <snt/exs/operator_base.h>
#include <snt/exs/token.h>
#include <snt/exs/token_list.h>
#include <snt/exs/token_list_base.h>

namespace snt::exs {

  // add
  
  class OperatorAdd : public OperatorBase {
  public:
    OperatorAdd(std::string s = "+");
    void operate_unary(TokenListBase* tokens) override;
    void operate_binary(TokenListBase* tokens) override;
  };

  // subtract
  
  class OperatorSubtract : public OperatorBase {
  public:
    
    OperatorSubtract(std::string s = "-");
    
    void operate_unary(TokenListBase* tokens) override;
    
    void operate_binary(TokenListBase* tokens) override;
  };

  // multiply

  class OperatorMultiply : public OperatorBase {
  public:
    
    OperatorMultiply(std::string s = "*");
    
    void operate_binary(TokenListBase* tokens) override;
  };

  // divide

  class OperatorDivide : public OperatorBase {
  public:
    OperatorDivide(std::string s = "/");
    void operate_binary(TokenListBase* tokens) override;
  };

  // modulo

  class OperatorModulo : public OperatorBase {
  public:
    
    OperatorModulo(std::string s = "%");
    
    void operate_binary(TokenListBase* tokens) override;
  };
  
} // namespace snt::exs

#endif // EXS_OPERATOR_ARITHMETIC_H
