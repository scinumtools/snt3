#ifndef EXS_OPERATOR_NOT_H
#define EXS_OPERATOR_NOT_H

#include <snt/exs/operator_base.h>

namespace snt::exs {

  class OperatorNot : public OperatorBase {
  public:
    OperatorNot(std::string s = "!");
    void operate_unary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_NOT_H
