#ifndef EXS_OPERATOR_OR_H
#define EXS_OPERATOR_OR_H

#include "../operator_base.h"

namespace snt::exs {

  class OperatorOr : public OperatorBase {
  public:
    OperatorOr(std::string s = "||");
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_OR_H
