#ifndef EXS_OPERATOR_TERNARY_H
#define EXS_OPERATOR_TERNARY_H

#include "settings.h"

#include <stdexcept>

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorTernary : public OperatorBase<A, S> {
  public:
    std::string symbol_other;
    OperatorTernary(
        std::string n,
        std::string s,
        std::string so,
        int t) : OperatorBase<A, S>(n, s, t), symbol_other(so) {}
    virtual void parse(Expression& expr) override {
      this->groups.clear();
      this->groups.reserve(expr.right.size());
      expr.remove(this->symbol);
      bool closed = false;
      while (expr.right.length() > 0) {
        if (expr.right.rfind(symbol_other, 0) == 0) {
          expr.remove(symbol_other);
          this->groups.push_back(expr.pop_left());
          closed = true;
          break;
        }
        expr.shift();
      }
      if (!closed) {
        throw std::logic_error("Incomplete ternary operator: " + expr.expr);
      }
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_TERNARY_H
