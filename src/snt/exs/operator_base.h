#ifndef EXS_OPERATOR_BASE_H
#define EXS_OPERATOR_BASE_H

#include "expression.h"
#include "settings.h"
#include "token_list_base.h"

#include <stdexcept>
#include <vector>

namespace snt::exs {

  class OperatorBase {
  public:
    std::string name;
    std::string symbol;
    int type;
    std::vector<std::string> groups;
    OperatorBase(std::string n, std::string s, int t) : name(n), symbol(s), type(t) {};
    virtual ~OperatorBase() = default;
    virtual bool check(Expression& expr);
    virtual void parse(Expression& expr);
    virtual void operate_unary(TokenListBase* tokens);
    virtual void operate_binary(TokenListBase* tokens);
    virtual void operate_ternary(TokenListBase* tokens);
    virtual void operate_group(TokenListBase* tokens);
    virtual void operate_unary(TokenListBase* tokens, BaseSettings* settings);
    virtual void operate_binary(TokenListBase* tokens, BaseSettings* settings);
    virtual void operate_ternary(TokenListBase* tokens, BaseSettings* settings);
    virtual void operate_group(TokenListBase* tokens, BaseSettings* settings);
    virtual void print() {};
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_BASE_H
