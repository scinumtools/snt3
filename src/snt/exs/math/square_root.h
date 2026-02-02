#ifndef EXS_OPERATOR_SQUARE_ROOT_H
#define EXS_OPERATOR_SQUARE_ROOT_H

#include "../operator_group.h"

namespace snt::exs {

  class OperatorSquareRoot : public OperatorGroup<1> {
  public:
    
    OperatorSquareRoot(const OperatorGroupSybols& s = {"sqrt", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_SQUARE_ROOT_H
