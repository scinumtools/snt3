#ifndef DIP_LOGICAL_OPERATORS_H
#define DIP_LOGICAL_OPERATORS_H

#include <snt/dip/environment.h>

namespace snt::dip {

    enum CustomOperatorType { DEFINED_OPERATOR = exs::NUM_OPERATOR_TYPES, NOT_DEFINED_OPERATOR };

    class OperatorDefined : public exs::OperatorGroup<1> {
      public:
        OperatorDefined();
        void operate_group(exs::TokenListBase* tokens);
    };

    class OperatorNotDefined : public exs::OperatorGroup<1> {
      public:
        OperatorNotDefined();
        void operate_group(exs::TokenListBase* tokens);
    };

} // namespace snt::dip

#endif // DIP_LOGICAL_OPERATORS_H
