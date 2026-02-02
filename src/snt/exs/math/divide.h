#ifndef EXS_OPERATOR_DIVIDE_H
#define EXS_OPERATOR_DIVIDE_H

#include "../operator_base.h"

namespace snt::exs {

  class OperatorDivide : public OperatorBase {
  public:
    OperatorDivide(std::string s = "/");
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_DIVIDE_H
