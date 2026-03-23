#ifndef EXS_OPERATOR_LOGICAL_H
#define EXS_OPERATOR_LOGICAL_H

#include <snt/exs/operator_base.h>

namespace snt::exs {

  // and

  class OperatorAnd : public OperatorBase {
  public:
    OperatorAnd(std::string s = "&&");
    void operate_binary(TokenListBase* tokens) override;
  };

  // or

  class OperatorOr : public OperatorBase {
  public:
    OperatorOr(std::string s = "||");
    void operate_binary(TokenListBase* tokens) override;
  };

  // not

  class OperatorNot : public OperatorBase {
  public:
    OperatorNot(std::string s = "!");
    void operate_unary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGICAL_H
