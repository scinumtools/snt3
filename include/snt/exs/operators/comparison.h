#ifndef EXS_OPERATOR_COMPARISON_H
#define EXS_OPERATOR_COMPARISON_H

#include <snt/exs/operator_base.h>

namespace snt::exs {

  // equal
  
  class OperatorEqual : public OperatorBase {
  public:
    
    OperatorEqual(std::string s = "==");
    
    void operate_binary(TokenListBase* tokens) override;
  };

  // not equal

  class OperatorNotEqual : public OperatorBase {
  public:
    
    OperatorNotEqual(std::string s = "!=");
    
    void operate_binary(TokenListBase* tokens) override;
  };

  // greater

  class OperatorGreater : public OperatorBase {
  public:
    
    OperatorGreater(std::string s = ">");
    
    void operate_binary(TokenListBase* tokens) override;
  };

  // less

  class OperatorLess : public OperatorBase {
  public:
    
    OperatorLess(std::string s = "<");
    
    void operate_binary(TokenListBase* tokens) override;
  };

  // greater equal

  class OperatorGreaterEqual : public OperatorBase {
  public:
    
    OperatorGreaterEqual(std::string s = ">=");
    
    void operate_binary(TokenListBase* tokens) override;
  };

  // less equal

  class OperatorLessEqual : public OperatorBase {
  public:
    
    OperatorLessEqual(std::string s = "<=");
    
    void operate_binary(TokenListBase* tokens) override;
  };
  
} // namespace snt::exs

#endif // EXS_OPERATOR_COMPARISON_H
