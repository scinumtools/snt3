#ifndef EXS_OPERATOR_BASE_H
#define EXS_OPERATOR_BASE_H

#include "expression.h"
#include "settings.h"
#include "token_list_base.h"

#include <stdexcept>
#include <vector>

namespace snt::exs {

  template <typename S = EmptySettings>
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
    virtual void operate_unary(TokenListBase* tokens) { throw std::logic_error("Unary operation is not implemented"); };
    virtual void operate_binary(TokenListBase* tokens) { throw std::logic_error("Binary operation is not implemented"); };
    virtual void operate_ternary(TokenListBase* tokens) { throw std::logic_error("Ternary operation is not implemented"); };
    virtual void operate_group(TokenListBase* tokens) { throw std::logic_error("Group operation is not implemented"); };
    virtual void operate_unary(TokenListBase* tokens, EmptySettings* settings) { operate_unary(tokens); };
    virtual void operate_binary(TokenListBase* tokens, EmptySettings* settings) { operate_binary(tokens); };
    virtual void operate_ternary(TokenListBase* tokens, EmptySettings* settings) { operate_ternary(tokens); };
    virtual void operate_group(TokenListBase* tokens, EmptySettings* settings) { operate_group(tokens); };
    virtual void print() {};
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_BASE_H
