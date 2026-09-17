#ifndef DIP_LOGICAL_OPERATORS_H
#define DIP_LOGICAL_OPERATORS_H

#include <snt/dip/environment.h>

namespace snt::dip {

    /** Operators used to test whether a DIP symbol is defined. */
    enum CustomOperatorType {
        DEFINED_OPERATOR = exs::NUM_OPERATOR_TYPES,
        NOT_DEFINED_OPERATOR
    };

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

} // namespace snt::dip

#endif // DIP_LOGICAL_OPERATORS_H
