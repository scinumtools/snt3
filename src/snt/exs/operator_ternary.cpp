#include <snt/exs/exceptions.h>
#include <snt/exs/operator_ternary.h>

namespace snt::exs {

    void OperatorTernary::parse(Expression& expr) {
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
            throw exs::ParserException(
                "Incomplete ternary operator",
                "The ternary expression is missing its closing `" + symbol_other + "` operator: `" + expr.expr + "`.",
                "Complete the ternary expression by providing the `" + symbol_other + "` operator.",
                __FILE__,
                __LINE__
            );
        }
    };

} // namespace snt::exs
