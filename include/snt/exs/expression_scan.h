#ifndef SNT_EXS_EXPRESSION_SCAN_H
#define SNT_EXS_EXPRESSION_SCAN_H

#include <snt/exs/expression.h>
#include <snt/exs/operator_list.h>
#include <string>
#include <utility>

namespace snt::exs::detail {

    /** Shared lexical scan for evaluated and structural expression paths. */
    template <bool STRUCTURAL = false, class Operand, class Group, class Operator>
    void scan_expression(
        const std::string& source,
        OperatorList& operators,
        Operand&& on_operand,
        Group&& on_group,
        Operator&& on_operator
    ) {
        Expression expression(source);
        while (!expression.right.empty()) {
            bool matched = false;
            for (const auto type : operators.order) {
                OperatorBase* op = operators.select(type);
                if (!op->check(expression))
                    continue;
                matched = true;
                std::string left = expression.pop_left();
                if (!left.empty())
                    on_operand(std::move(left));
                if constexpr (STRUCTURAL)
                    op->parse_for_composition(expression);
                else
                    op->parse(expression);
                const auto groups = op->groups;
                for (const auto& group : groups)
                    on_group(group);
                on_operator(type, groups.size());
            }
            if (!matched)
                expression.shift();
        }
        std::string left = expression.pop_left();
        if (!left.empty())
            on_operand(std::move(left));
    }

} // namespace snt::exs::detail

#endif // SNT_EXS_EXPRESSION_SCAN_H
