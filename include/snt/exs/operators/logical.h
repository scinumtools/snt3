#ifndef EXS_OPERATOR_LOGICAL_H
#define EXS_OPERATOR_LOGICAL_H

#include <snt/exs/operator_base.h>

namespace snt::exs {

    /** Logical AND operator. */
    class OperatorAnd : public OperatorBase {
      public:
        /** Construct a logical AND operator. */
        OperatorAnd(std::string s = "&&");
        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    /** Logical OR operator. */
    class OperatorOr : public OperatorBase {
      public:
        /** Construct a logical OR operator. */
        OperatorOr(std::string s = "||");
        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    /** Logical NOT operator. */
    class OperatorNot : public OperatorBase {
      public:
        /** Construct a logical NOT operator. */
        OperatorNot(std::string s = "!");
        /** @copydoc OperatorBase::operate_unary */
        void operate_unary(TokenListBase* tokens) override;
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGICAL_H
