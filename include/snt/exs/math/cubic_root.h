#ifndef EXS_OPERATOR_CUBIC_ROOT_H
#define EXS_OPERATOR_CUBIC_ROOT_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

  class OperatorCubicRoot : public OperatorGroup<1> {
  public:
    OperatorCubicRoot(const OperatorGroupSybols& s = {"cbrt", "(", ")", ","});
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_CUBIC_ROOT_H
