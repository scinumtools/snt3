#ifndef EXS_OPERATOR_CONTROL_H
#define EXS_OPERATOR_CONTROL_H

#include <snt/exs/operator_group.h>
#include <snt/exs/operator_ternary.h>

namespace snt::exs {

    /** Parenthesized expression grouping operator. */
    class OperatorParentheses : public OperatorGroup<1> {
      public:
        /** Construct a parenthesis grouping operator. */
        OperatorParentheses(const OperatorGroupSybols& s = {"", "(", ")", ","});
    };

    /** Ternary conditional operator. */
    class OperatorCondition : public OperatorTernary {
      public:
        /** Construct a conditional operator. */
        OperatorCondition();
        /** @copydoc OperatorTernary::operate_ternary */
        void operate_ternary(TokenListBase* tokens) override;
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_CONTROL_H
