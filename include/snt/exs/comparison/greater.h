#ifndef EXS_OPERATOR_GREATER_H
#define EXS_OPERATOR_GREATER_H

#include <snt/exs/operator_base.h>

namespace snt::exs {

  class OperatorGreater : public OperatorBase {
  public:
    
    OperatorGreater(std::string s = ">");
    
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_GREATER_H
