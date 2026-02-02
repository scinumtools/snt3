#ifndef EXS_OPERATOR_EQUAL_H
#define EXS_OPERATOR_EQUAL_H

#include "../operator_base.h"

namespace snt::exs {

  class OperatorEqual : public OperatorBase {
  public:
    
    OperatorEqual(std::string s = "==");
    
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_EQUAL_H
