#ifndef EXS_OPERATOR_SUBTRACT_H
#define EXS_OPERATOR_SUBTRACT_H

#include "../operator_base.h"
#include "../token.h"
#include "../token_list.h"
#include "../token_list_base.h"

namespace snt::exs {

  class OperatorSubtract : public OperatorBase {
  public:
    
    OperatorSubtract(std::string s = "-");
    
    void operate_unary(TokenListBase* tokens) override;
    
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_SUBTRACT_H
