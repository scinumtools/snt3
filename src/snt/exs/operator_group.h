#ifndef EXS_OPERATOR_GROUP_H
#define EXS_OPERATOR_GROUP_H

#include "operator_base.h"
#include "settings.h"

#include <stdexcept>

namespace snt::exs {

  struct OperatorGroupSybols {
    std::string prefix = "";
    std::string open = "(";
    std::string close = ")";
    std::string separator = ",";
  };

  template <int N = 0, typename S = EmptySettings>
  class OperatorGroup : public OperatorBase<S> {
  public:
    size_t num_groups = N;
    OperatorGroupSybols symbols;
    OperatorGroup(const std::string& n, const OperatorGroupSybols& s, const int t) : OperatorBase<S>(n, s.prefix + s.open, t), symbols(s) {}
    virtual void parse(Expression& expr) override {
      this->groups.clear();
      expr.remove(this->symbol);
      int depth = 1;
      while (depth > 0) {
        if (expr.right.length() == 0) {
          throw std::logic_error("Unclosed parentheses: " + expr.expr);
        } else if (expr.right.rfind(this->symbol, 0) == 0 || expr.right.rfind(symbols.open, 0) == 0) {
          depth++;
        } else if (expr.right.rfind(symbols.separator, 0) == 0 && depth == 1) {
          expr.remove(symbols.separator);
          this->groups.push_back(expr.pop_left());
        } else if (expr.right.rfind(symbols.close, 0) == 0) {
          depth--;
          if (depth == 0) {
            expr.remove(symbols.close);
            this->groups.push_back(expr.pop_left());
            break;
          }
        }
        expr.shift();
      }
      num_groups = this->groups.size();
      if (N > 0 && num_groups != N) {
        throw std::logic_error("Wrong number of group members: " + std::to_string(this->groups.size()) + ", " + std::to_string(N));
      }
    };
    virtual void operate_group(TokenListBase* tokens) override {}; // this should stay empty
    virtual void operate_group(TokenListBase* tokens, S* settings) override { operate_group(tokens); };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_GROUP_H
