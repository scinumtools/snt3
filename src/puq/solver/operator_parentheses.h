#ifndef PUQ_OPERATOR_PARENTHESES_H
#define PUQ_OPERATOR_PARENTHESES_H

#include "../../exs/pch_exs.h"
#include "unit_atom.h"

namespace puq {

  class OperatorParentheses : public exs::OperatorGroup<UnitAtom, 1> {
  public:
    std::vector<EXPONENT_TYPE> exponent;
    OperatorParentheses() : OperatorGroup<UnitAtom, 1>("par", {"", "(", ")", ","}, exs::PARENTHESES_OPERATOR) {}
    virtual bool check(exs::Expression& expr);
    virtual void parse(exs::Expression& expr);
    void operate_group(exs::TokenListBase<UnitAtom>* tokens);
  };
  
} // namespace puq

#endif // PUQ_OPERATOR_PARENTHESES_H
