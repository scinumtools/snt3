#ifndef EXS_OPERATOR_EXPONENTIAL_H
#define EXS_OPERATOR_EXPONENTIAL_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

    /** Exponential function operator. */
    class OperatorExponent : public OperatorGroup<1> {
      public:
        OperatorExponent(const OperatorGroupSybols& s = {"exp", "(", ")", ","});

        /** @copydoc OperatorGroup<1>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

    /** Natural logarithm operator. */
    class OperatorLogarithm : public OperatorGroup<1> {
      public:
        OperatorLogarithm(const OperatorGroupSybols& s = {"log", "(", ")", ","});

        /** @copydoc OperatorGroup<1>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

    /** Base-ten logarithm operator. */
    class OperatorLogarithm10 : public OperatorGroup<1> {
      public:
        OperatorLogarithm10(const OperatorGroupSybols& s = {"log10", "(", ")", ","});

        /** @copydoc OperatorGroup<1>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

    /** Arbitrary-base logarithm operator. */
    class OperatorLogarithmBase : public OperatorGroup<2> {
      public:
        OperatorLogarithmBase(const OperatorGroupSybols& s = {"logb", "(", ")", ","});

        /** @copydoc OperatorGroup<2>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

    /** Square-root function operator. */
    class OperatorSquareRoot : public OperatorGroup<1> {
      public:
        OperatorSquareRoot(const OperatorGroupSybols& s = {"sqrt", "(", ")", ","});

        /** @copydoc OperatorGroup<1>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

    /** Cube-root function operator. */
    class OperatorCubicRoot : public OperatorGroup<1> {
      public:
        OperatorCubicRoot(const OperatorGroupSybols& s = {"cbrt", "(", ")", ","});
        /** @copydoc OperatorGroup<1>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

    /** Binary power operator. */
    class OperatorPower : public OperatorBase {
      public:
        OperatorPower(std::string s = "**");

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    /** Arbitrary-base power function operator. */
    class OperatorPowerBase : public OperatorGroup<2> {
      public:
        OperatorPowerBase(const OperatorGroupSybols& s = {"powb", "(", ")", ","});

        /** @copydoc OperatorGroup<2>::operate_group */
        void operate_group(TokenListBase* tokens) override;
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_EXPONENTIAL_H
