#ifndef EXS_OPERATOR_BASE_H
#define EXS_OPERATOR_BASE_H

#include <snt/exs/expression.h>
#include <snt/exs/settings.h>
#include <snt/exs/token_list_base.h>
#include <stdexcept>
#include <vector>

namespace snt::exs {

    /** Base interface for an EXS operator. */
    class OperatorBase {
      public:
        std::string name;
        std::string symbol;
        int type;
        std::vector<std::string> groups;
        /** Create an operator with a name, symbol, and precedence.
         * @param n Operator or object name.
         * @param s Operator symbol or source string.
         * @param t Operator type or precedence.
         */
        OperatorBase(std::string n, std::string s, int t) : name(std::move(n)), symbol(std::move(s)), type(t) {};
        virtual ~OperatorBase() = default;
        /** Check whether the expression supports this operator.
         * @param expr Expression being parsed.
         */
        virtual bool check(Expression& expr);
        /** Parse the supplied expression.
         * @param expr Expression being parsed.
         */
        virtual void parse(Expression& expr);
        /** Apply the unary operator to the token list.
         * @param tokens Token list being processed.
         */
        virtual void operate_unary(TokenListBase* tokens);
        /** Apply the binary operator to the token list.
         * @param tokens Token list being processed.
         */
        virtual void operate_binary(TokenListBase* tokens);
        /** Apply the ternary operator to the token list.
         * @param tokens Token list being processed.
         */
        virtual void operate_ternary(TokenListBase* tokens);
        /** Apply the grouped operator to the token list.
         * @param tokens Token list being processed.
         */
        virtual void operate_group(TokenListBase* tokens);
        /** Apply the unary operator to the token list.
         * @param tokens Token list being processed.
         * @param settings Operator or parser settings.
         */
        virtual void operate_unary(TokenListBase* tokens, BaseSettings* settings);
        /** Apply the binary operator to the token list.
         * @param tokens Token list being processed.
         * @param settings Operator or parser settings.
         */
        virtual void operate_binary(TokenListBase* tokens, BaseSettings* settings);
        /** Apply the ternary operator to the token list.
         * @param tokens Token list being processed.
         * @param settings Operator or parser settings.
         */
        virtual void operate_ternary(TokenListBase* tokens, BaseSettings* settings);
        /** Apply the grouped operator to the token list.
         * @param tokens Token list being processed.
         * @param settings Operator or parser settings.
         */
        virtual void operate_group(TokenListBase* tokens, BaseSettings* settings);
        virtual void print() {};
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_BASE_H
