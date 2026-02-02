#ifndef PUQ_OPERATOR_PARENTHESES_H
#define PUQ_OPERATOR_PARENTHESES_H

#include "../../exs.h"
#include "unit_atom.h"

namespace snt::puq {

  class OperatorParentheses : public exs::OperatorGroup<1> {
  public:
    std::vector<EXPONENT_TYPE> exponent;
    OperatorParentheses() : OperatorGroup<1>("par", {"", "(", ")", ","}, exs::PARENTHESES_OPERATOR) {}
    virtual bool check(exs::Expression& expr);
    virtual void parse(exs::Expression& expr);
    void operate_group(exs::TokenListBase* tokens);
  };

} // namespace snt::puq

#endif // PUQ_OPERATOR_PARENTHESES_H
