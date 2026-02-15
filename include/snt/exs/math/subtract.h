#ifndef EXS_OPERATOR_SUBTRACT_H
#define EXS_OPERATOR_SUBTRACT_H

#include <snt/exs/operator_base.h>
#include <snt/exs/token.h>
#include <snt/exs/token_list.h>
#include <snt/exs/token_list_base.h>

namespace snt::exs {

  class OperatorSubtract : public OperatorBase {
  public:
    
    OperatorSubtract(std::string s = "-");
    
    void operate_unary(TokenListBase* tokens) override;
    
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_SUBTRACT_H
