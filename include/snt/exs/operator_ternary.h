#ifndef EXS_OPERATOR_TERNARY_H
#define EXS_OPERATOR_TERNARY_H

#include <snt/exs/operator_base.h>
#include <snt/exs/settings.h>
#include <stdexcept>

namespace snt::exs {

    /** EXS operator with three input branches, such as a conditional. */
    class OperatorTernary : public OperatorBase {
      public:
        std::string symbol_other;
        /** Create a ternary operator.
         * @param n Operator or object name.
         * @param s Operator symbol or source string.
         * @param so Secondary operator symbol.
         * @param t Operator type or precedence.
         */
        OperatorTernary(std::string n, std::string s, std::string so, int t)
            : OperatorBase(n, s, t), symbol_other(so) {}
        virtual void parse(Expression& expr) override;
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_TERNARY_H
