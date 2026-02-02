#ifndef EXS_OPERATOR_COSINUS_H
#define EXS_OPERATOR_COSINUS_H

#include "../operator_group.h"

namespace snt::exs {

  class OperatorCosinus : public OperatorGroup<1> {
  public:
    OperatorCosinus(const OperatorGroupSybols& s = {"cos", "(", ")", ","});
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_COSINUS_H
