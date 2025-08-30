#ifndef EXS_OPERATOR_BASE_H
#define EXS_OPERATOR_BASE_H

#include "settings.h"
#include "expression.h"
#include "token_list_base.h"

#include <stdexcept>

namespace exs {
  
  template <class A, typename S = EmptySettings>
  class OperatorBase {
  public:
    std::string name;
    std::string symbol;
    int type;
    std::vector<std::string> groups;
    OperatorBase(std::string n, std::string s, int t) : name(n), symbol(s), type(t) {};
    virtual ~OperatorBase() = default;
    virtual bool check(Expression& expr) {
      return expr.right.rfind(symbol, 0) == 0;
    }
    virtual void parse(Expression& expr) {
      if (expr.right.length() > 0) {
        expr.remove(symbol);
      }
    };
    virtual void operate_unary(TokenListBase<A>* tokens) { throw std::logic_error("Unary operation is not implemented"); };
    virtual void operate_binary(TokenListBase<A>* tokens) { throw std::logic_error("Binary operation is not implemented"); };
    virtual void operate_ternary(TokenListBase<A>* tokens) { throw std::logic_error("Ternary operation is not implemented"); };
    virtual void operate_group(TokenListBase<A>* tokens) { throw std::logic_error("Group operation is not implemented"); };
    virtual void operate_unary(TokenListBase<A>* tokens, S* settings) { operate_unary(tokens); };
    virtual void operate_binary(TokenListBase<A>* tokens, S* settings) { operate_binary(tokens); };
    virtual void operate_ternary(TokenListBase<A>* tokens, S* settings) { operate_ternary(tokens); };
    virtual void operate_group(TokenListBase<A>* tokens, S* settings) { operate_group(tokens); };
    virtual void print() {};
  };

} // namespace exs

#endif // EXS_OPERATOR_BASE_H
