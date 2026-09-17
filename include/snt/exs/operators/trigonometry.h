#ifndef EXS_OPERATOR_TRIGONOMETRY_H
#define EXS_OPERATOR_TRIGONOMETRY_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

    /** Sine function operator. */
    class OperatorSinus : public OperatorGroup<1> {
      public:
        OperatorSinus(const OperatorGroupSybols& s = {"sin", "(", ")", ","});

        /** @copydoc OperatorGroup<1>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

    /** Cosine function operator. */
    class OperatorCosinus : public OperatorGroup<1> {
      public:
        OperatorCosinus(const OperatorGroupSybols& s = {"cos", "(", ")", ","});
        /** @copydoc OperatorGroup<1>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

    /** Tangent function operator. */
    class OperatorTangens : public OperatorGroup<1> {
      public:
        OperatorTangens(const OperatorGroupSybols& s = {"tan", "(", ")", ","});

        /** @copydoc OperatorGroup<1>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_TRIGONOMETRY_H
