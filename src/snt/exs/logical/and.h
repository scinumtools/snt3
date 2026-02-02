#ifndef EXS_OPERATOR_AND_H
#define EXS_OPERATOR_AND_H

#include "../operator_base.h"

namespace snt::exs {

  class OperatorAnd : public OperatorBase {
  public:
    OperatorAnd(std::string s = "&&");
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_AND_H
