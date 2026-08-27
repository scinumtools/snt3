#ifndef EXS_OPERATOR_GROUP_H
#define EXS_OPERATOR_GROUP_H

#include <snt/exs/exceptions.h>
#include <snt/exs/operator_base.h>
#include <snt/exs/settings.h>
#include <stdexcept>

namespace snt::exs {

    struct OperatorGroupSybols {
        std::string prefix = "";
        std::string open = "(";
        std::string close = ")";
        std::string separator = ",";
    };

    /**
     * Group operator class
     *
     * @tparam N Number of group members. Zero means unlimited number
     */
    template <int N = 0> class OperatorGroup : public OperatorBase {
      public:
        size_t num_groups = N;
        OperatorGroupSybols symbols;
        OperatorGroup(const std::string& n, const OperatorGroupSybols& s, const int t)
            : OperatorBase(n, s.prefix + s.open, t), symbols(s) {}
        virtual void parse(Expression& expr) override {
            this->groups.clear();
            expr.remove(this->symbol);
            int depth = 1;
            while (depth > 0) {
                if (expr.right.length() == 0) {
                    throw exs::ParserException(
                        "Unclosed parentheses",
                        "The expression is missing a closing parenthesis: `" + expr.expr + "`.",
                        "Add a closing `)` parenthesis to complete the expression.",
                        __FILE__,
                        __LINE__
                    );
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
                throw exs::ParserException(
                    "Invalid number of groups",
                    "The expression contains `" + std::to_string(num_groups) + "` groups, but `" + std::to_string(N) +
                        "` are required: `" + expr.expr + "`.",
                    "Provide exactly `" + std::to_string(N) + "` groups.",
                    __FILE__,
                    __LINE__
                );
            }
        };
        virtual void operate_group(TokenListBase* tokens) override {}; // this should stay empty
        virtual void operate_group(TokenListBase* tokens, BaseSettings* settings) override { operate_group(tokens); };
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_GROUP_H
