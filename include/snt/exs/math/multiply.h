#ifndef EXS_OPERATOR_MULTIPLY_H
#define EXS_OPERATOR_MULTIPLY_H

#include <snt/exs/operator_base.h>

namespace snt::exs {

  class OperatorMultiply : public OperatorBase {
  public:
    
    OperatorMultiply(std::string s = "*");
    
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_MULTIPLY_H
