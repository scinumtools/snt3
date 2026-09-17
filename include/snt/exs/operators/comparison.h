#ifndef EXS_OPERATOR_COMPARISON_H
#define EXS_OPERATOR_COMPARISON_H

#include <snt/exs/operator_base.h>

namespace snt::exs {

    /** Equality comparison operator. */
    class OperatorEqual : public OperatorBase {
      public:
        /** Construct an equality operator. */
        OperatorEqual(std::string s = "==");

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    /** Inequality comparison operator. */
    class OperatorNotEqual : public OperatorBase {
      public:
        /** Construct an inequality operator. */
        OperatorNotEqual(std::string s = "!=");

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    /** Greater-than comparison operator. */
    class OperatorGreater : public OperatorBase {
      public:
        /** Construct a greater-than operator. */
        OperatorGreater(std::string s = ">");

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    /** Less-than comparison operator. */
    class OperatorLess : public OperatorBase {
      public:
        /** Construct a less-than operator. */
        OperatorLess(std::string s = "<");

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    /** Greater-than-or-equal comparison operator. */
    class OperatorGreaterEqual : public OperatorBase {
      public:
        /** Construct a greater-than-or-equal operator. */
        OperatorGreaterEqual(std::string s = ">=");

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

    /** Less-than-or-equal comparison operator. */
    class OperatorLessEqual : public OperatorBase {
      public:
        /** Construct a less-than-or-equal operator. */
        OperatorLessEqual(std::string s = "<=");

        /** @copydoc OperatorBase::operate_binary */
        void operate_binary(TokenListBase* tokens) override;
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_COMPARISON_H
