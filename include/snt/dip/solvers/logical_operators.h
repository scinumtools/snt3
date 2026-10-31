#ifndef DIP_LOGICAL_OPERATORS_H
#define DIP_LOGICAL_OPERATORS_H

#include <snt/dip/environment.h>

namespace snt::dip {

    /** Operators used to test whether a DIP symbol is defined. */
    enum CustomOperatorType {
        DEFINED_OPERATOR = exs::NUM_OPERATOR_TYPES,
        NOT_DEFINED_OPERATOR,
        HAS_NAN_OPERATOR,
        HAS_INF_OPERATOR,
        HAS_POS_INF_OPERATOR,
        HAS_NEG_INF_OPERATOR,
        HAS_POSITIVE_OPERATOR,
        HAS_NEGATIVE_OPERATOR,
        HAS_ZERO_OPERATOR,
        ALL_FINITE_OPERATOR
    };

    /** Classification applied to the evaluated numerical value of an operand. */
    enum class NumericPredicate { Nan, Inf, PosInf, NegInf, Positive, Negative, Zero, AllFinite };

    /** DIP operator that tests for a defined symbol. */
    class OperatorDefined : public exs::OperatorGroup<1> {
      public:
        /** Construct the defined-symbol operator. */
        OperatorDefined();
        /** @copydoc exs::OperatorGroup<1>::operate_group */
        void operate_group(exs::TokenListBase* tokens) override;
    };

    /** DIP operator that tests for an undefined symbol. */
    class OperatorNotDefined : public exs::OperatorGroup<1> {
      public:
        /** Construct the undefined-symbol operator. */
        OperatorNotDefined();
        /** @copydoc exs::OperatorGroup<1>::operate_group */
        void operate_group(exs::TokenListBase* tokens) override;
    };

    /** Unary numerical predicate returning a scalar boolean. */
    class OperatorNumericPredicate : public exs::OperatorGroup<1> {
      public:
        OperatorNumericPredicate(const char* name, NumericPredicate predicate, CustomOperatorType type);
        void operate_group(exs::TokenListBase* tokens) override;

      private:
        NumericPredicate predicate_;
    };

} // namespace snt::dip

#endif // DIP_LOGICAL_OPERATORS_H
