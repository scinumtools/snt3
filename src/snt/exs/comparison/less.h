#ifndef EXS_OPERATOR_LESS_H
#define EXS_OPERATOR_LESS_H

#include "../operator_base.h"

namespace snt::exs {

  class OperatorLess : public OperatorBase {
  public:
    
    OperatorLess(std::string s = "<");
    
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LESS_H
