#ifndef EXS_OPERATOR_ARITHMETIC_H
#define EXS_OPERATOR_ARITHMETIC_H

#include <snt/exs/operator_base.h>
#include <snt/exs/token.h>
#include <snt/exs/token_list.h>
#include <snt/exs/token_list_base.h>

namespace snt::exs {

    // add

    /** Binary and unary addition operator. */
    class OperatorAdd : public OperatorBase {
      public:
        /** Construct an addition operator with the supplied symbol. */
        OperatorAdd(std::string s = "+");
        /** @copydoc OperatorBase::operate_unary */
        void operate_unary(TokenListBase* tokens) override;
        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    // subtract

    /** Binary and unary subtraction operator. */
    class OperatorSubtract : public OperatorBase {
      public:
        /** Construct a subtraction operator with the supplied symbol. */
        OperatorSubtract(std::string s = "-");

        /** @copydoc OperatorBase::operate_unary */
        void operate_unary(TokenListBase* tokens) override;

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    // multiply

    /** Multiplication operator. */
    class OperatorMultiply : public OperatorBase {
      public:
        /** Construct a multiplication operator with the supplied symbol. */
        OperatorMultiply(std::string s = "*");

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    // divide

    /** Division operator. */
    class OperatorDivide : public OperatorBase {
      public:
        /** Construct a division operator with the supplied symbol. */
        OperatorDivide(std::string s = "/");
        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    // modulo

    /** Remainder operator. */
    class OperatorModulo : public OperatorBase {
      public:
        /** Construct a remainder operator with the supplied symbol. */
        OperatorModulo(std::string s = "%");

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_ARITHMETIC_H
