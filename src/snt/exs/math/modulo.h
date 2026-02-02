#ifndef EXS_OPERATOR_MODULO_H
#define EXS_OPERATOR_MODULO_H

#include "../operator_base.h"

namespace snt::exs {

  class OperatorModulo : public OperatorBase {
  public:
    
    OperatorModulo(std::string s = "%");
    
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_MODULO_H
