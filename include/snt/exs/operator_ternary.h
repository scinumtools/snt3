#ifndef EXS_OPERATOR_TERNARY_H
#define EXS_OPERATOR_TERNARY_H

#include <snt/exs/settings.h>
#include <snt/exs/operator_base.h>

#include <stdexcept>

namespace snt::exs {

  class OperatorTernary : public OperatorBase {
  public:
    std::string symbol_other;
    OperatorTernary(
        std::string n,
        std::string s,
        std::string so,
        int t) : OperatorBase(n, s, t), symbol_other(so) {}
    virtual void parse(Expression& expr) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_TERNARY_H
