#ifndef EXS_OPERATOR_GREATER_EQUAL_H
#define EXS_OPERATOR_GREATER_EQUAL_H

#include "../operator_base.h"

namespace snt::exs {

  class OperatorGreaterEqual : public OperatorBase {
  public:
    
    OperatorGreaterEqual(std::string s = ">=");
    
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_GREATER_EQUAL_H
