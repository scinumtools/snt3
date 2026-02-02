#include "operator_base.h"

namespace snt::exs {

  bool OperatorBase::check(Expression& expr) {
    return expr.right.rfind(symbol, 0) == 0;
  }
  
  void OperatorBase::parse(Expression& expr) {
    if (expr.right.length() > 0) {
      expr.remove(symbol);
    }
  };
  
  void OperatorBase::operate_unary(TokenListBase* tokens) {
    throw std::logic_error("Unary operation is not implemented");
  };
  
  void OperatorBase::operate_binary(TokenListBase* tokens) {
    throw std::logic_error("Binary operation is not implemented");
  };
  
  void OperatorBase::operate_ternary(TokenListBase* tokens) {
    throw std::logic_error("Ternary operation is not implemented");
  };
  
  void OperatorBase::operate_group(TokenListBase* tokens) {
    throw std::logic_error("Group operation is not implemented");
  };
  
  void OperatorBase::operate_unary(TokenListBase* tokens, BaseSettings* settings) {
    operate_unary(tokens);
  };
  
  void OperatorBase::operate_binary(TokenListBase* tokens, BaseSettings* settings) {
    operate_binary(tokens);
  };
  
  void OperatorBase::operate_ternary(TokenListBase* tokens, BaseSettings* settings) {
    operate_ternary(tokens);
  };
  
  void OperatorBase::operate_group(TokenListBase* tokens, BaseSettings* settings) {
    operate_group(tokens);
  };
  
}
