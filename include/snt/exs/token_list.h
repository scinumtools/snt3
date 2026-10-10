#ifndef EXS_TOKEN_LIST_H
#define EXS_TOKEN_LIST_H

#include <algorithm>
#include <deque>
#include <iostream>
#include <snt/exs/atom_list.h>
#include <snt/exs/composition.h>
#include <snt/exs/exceptions.h>
#include <snt/exs/operator_list.h>
#include <snt/exs/settings.h>
#include <snt/exs/token.h>
#include <snt/exs/token_list.h>
#include <stdexcept>
#include <vector>

namespace snt::exs {

    /** Collection of tokens used during expression parsing. */
    class TokenList : public TokenListBase {
      public:
        std::deque<Token> left;
        std::deque<Token> right;
        OperatorList* operators;
        BaseSettings* settings;
        AtomList atoms;
        CompositionGraph* composition = nullptr;
        TokenList(OperatorList* o, BaseSettings* set = nullptr, CompositionGraph* graph = nullptr)
            : operators(o), settings(set), composition(graph) {};
        /** Append a token with no integer payload.
         * @param t Token category.
         */
        void append(TokenType t) { right.push_back(Token(t)); };
        /** Append a token carrying an integer payload.
         * @param t Token category.
         * @param o Integer token payload, such as an operator index.
         */
        void append(TokenType t, int o) { right.push_back(Token(t, o)); };
        void append(TokenType t, int o, std::size_t group_count) {
            Token token(t, o);
            token.group_count = group_count;
            right.push_back(std::move(token));
        }
        // void append(TokenType t, std::string s) {
        /** Append a token that owns an expression atom.
         * @param t Token category.
         * @param at Atom stored in the token list.
         */
        //   AtomGrand* a = atoms.append(s, settings);
        //   right.push_back(Token(t, a));
        // };
        void append(TokenType t, std::unique_ptr<AtomGrand> at) {
            AtomGrand* a = atoms.append(std::move(at));
            right.push_back(Token(t, a));
        };
        void append(TokenType t, std::unique_ptr<AtomGrand> at, std::size_t node) {
            append(t, std::move(at));
            right.back().composition_node = node;
        }
        Token get_left() override {
            if (left.empty()) {
                return Token(EMPTY_TOKEN);
            } else {
                Token t = left.back();
                left.pop_back();
                return t;
            }
        };
        Token get_right() override {
            if (right.empty()) {
                return Token(EMPTY_TOKEN);
            } else {
                Token t = right.front();
                right.pop_front();
                return t;
            }
        };
        void put_left(Token t) override {
            if (t.type != EMPTY_TOKEN) {
                left.push_back(t);
            }
        };
        void put_right(Token t) override {
            if (t.type != EMPTY_TOKEN) {
                right.push_front(t);
            }
        };
        void operate(const std::vector<int>& ops, OperationType oitype) {
            /*
            std::cout << "token_list::operate oitype=" << oitype << " operators=[ ";
            for (auto i: ops) std::cout << i << " ";
            std::cout << "]" << std::endl;
            print(true);
            */
            // perform operations on the individual tokens
            while (!right.empty()) {
                Token token = get_right();
                // token.print();
                if (std::find(ops.begin(), ops.end(), token.optype) != ops.end()) {
                    OperatorBase* op = operators->select(token.optype);
                    std::vector<std::size_t> children;
                    std::vector<int> following_prefix;
                    AtomGrand* result_atom = nullptr;
                    bool fold_signs = false;
                    if (composition) {
                        const auto add_child = [&](const Token& item) {
                            if (item.type == ATOM_TOKEN && item.composition_node)
                                children.push_back(*item.composition_node);
                        };
                        if (oitype == GROUP_OPERATION) {
                            if (left.size() >= token.group_count) {
                                for (std::size_t i = left.size() - token.group_count; i < left.size(); ++i)
                                    add_child(left[i]);
                                if (token.group_count)
                                    result_atom = left.back().atom;
                            }
                        } else if (oitype == BINARY_OPERATION && !left.empty() && !right.empty()) {
                            add_child(left.back());
                            add_child(right.front());
                            result_atom = left.back().atom;
                        } else if (oitype == TERNARY_OPERATION && left.size() >= 2 && !right.empty()) {
                            add_child(left[left.size() - 2]);
                            add_child(left.back());
                            add_child(right.front());
                            result_atom = left[left.size() - 2].atom;
                        } else if (oitype == UNARY_OPERATION && !right.empty()) {
                            const bool prefix = left.empty() || left.back().type != ATOM_TOKEN;
                            if (prefix && right.front().type == ATOM_TOKEN) {
                                add_child(right.front());
                                result_atom = right.front().atom;
                            }
                            if (right.front().type == OPERATOR_TOKEN &&
                                (token.optype == ADD_OPERATOR || token.optype == SUBTRACT_OPERATOR) &&
                                (right.front().optype == ADD_OPERATOR || right.front().optype == SUBTRACT_OPERATOR)) {
                                fold_signs = true;
                                following_prefix = right.front().prefix_operators;
                                if (following_prefix.empty())
                                    following_prefix.push_back(right.front().optype);
                            }
                        }
                    }
                    // token is an operator
                    if (oitype == UNARY_OPERATION) {
                        op->operate_unary(this, settings);
                    } else if (oitype == BINARY_OPERATION) {
                        if (left.empty() || left.back().type != ATOM_TOKEN || right.empty() ||
                            right.front().type != ATOM_TOKEN) {
                            throw exs::ParserException(
                                "Invalid expression syntax",
                                "The operator `" + op->symbol + "` needs an operand on each side.",
                                "Check for missing values or consecutive operators in the expression.",
                                __FILE__,
                                __LINE__
                            );
                        }
                        op->operate_binary(this, settings);
                    } else if (oitype == TERNARY_OPERATION) {
                        op->operate_ternary(this, settings);
                    } else if (oitype == GROUP_OPERATION) {
                        op->operate_group(this, settings);
                    } else {
                        throw exs::ParserException(
                            "Invalid operation type",
                            "The operation type `" + std::to_string(oitype) + "` is not supported.",
                            "Use a valid unary, binary, ternary, or group operation type.",
                            __FILE__,
                            __LINE__
                        );
                    }
                    if (composition) {
                        if (fold_signs && !right.empty() && right.front().type == OPERATOR_TOKEN) {
                            std::vector<int> prefixes = token.prefix_operators;
                            if (prefixes.empty())
                                prefixes.push_back(token.optype);
                            auto& next = right.front();
                            prefixes.insert(prefixes.end(), following_prefix.begin(), following_prefix.end());
                            next.prefix_operators = std::move(prefixes);
                        } else if (
                            oitype == UNARY_OPERATION && !token.prefix_operators.empty() && !left.empty() &&
                            left.back().type == OPERATOR_TOKEN
                        ) {
                            left.back().prefix_operators = std::move(token.prefix_operators);
                        } else if (!children.empty() && result_atom) {
                            Token* result = nullptr;
                            if (!left.empty() && left.back().type == ATOM_TOKEN && left.back().atom == result_atom)
                                result = &left.back();
                            else if (
                                !right.empty() && right.front().type == ATOM_TOKEN && right.front().atom == result_atom
                            )
                                result = &right.front();
                            else if (!left.empty() && left.back().type == ATOM_TOKEN)
                                result = &left.back();
                            else if (!right.empty() && right.front().type == ATOM_TOKEN)
                                result = &right.front();
                            if (result) {
                                std::size_t node = children.front();
                                if (oitype == UNARY_OPERATION && !token.prefix_operators.empty()) {
                                    for (auto i = token.prefix_operators.rbegin(); i != token.prefix_operators.rend();
                                         ++i) {
                                        const auto* prefix = operators->select(*i);
                                        const auto child = node;
                                        node = composition->nodes.size();
                                        composition->nodes.push_back(
                                            {CompositionKind::Operator, prefix->name, *i, UNARY_OPERATION, {child}}
                                        );
                                    }
                                    result->composition_node = node;
                                } else if (
                                    oitype == BINARY_OPERATION && !token.prefix_operators.empty() &&
                                    children.size() == 2
                                ) {
                                    const auto left_node = children[0];
                                    node = children[1];
                                    for (auto i = token.prefix_operators.rbegin();
                                         i + 1 != token.prefix_operators.rend();
                                         ++i) {
                                        const auto* prefix = operators->select(*i);
                                        const auto child = node;
                                        node = composition->nodes.size();
                                        composition->nodes.push_back(
                                            {CompositionKind::Operator, prefix->name, *i, UNARY_OPERATION, {child}}
                                        );
                                    }
                                    const auto outer_type = token.prefix_operators.front();
                                    const auto* outer = operators->select(outer_type);
                                    const auto right_node = node;
                                    node = composition->nodes.size();
                                    composition->nodes.push_back(
                                        {CompositionKind::Operator,
                                         outer->name,
                                         outer_type,
                                         BINARY_OPERATION,
                                         {left_node, right_node}}
                                    );
                                    result->composition_node = node;
                                } else {
                                    node = composition->nodes.size();
                                    composition->nodes.push_back(
                                        {oitype == GROUP_OPERATION ? CompositionKind::Group : CompositionKind::Operator,
                                         op->name,
                                         token.optype,
                                         oitype,
                                         std::move(children)}
                                    );
                                    result->composition_node = node;
                                }
                            }
                        }
                    }
                } else {
                    // token is something else
                    // std::cout << "putting left" << std::endl;
                    put_left(token);
                }
                // print(true);
            }
            // move all tokens from left to right
            left.swap(right);
            // print(true);
            // std::cout << std::endl;
        };
        /** Format the object as text.
         * @param details Detailed diagnostic explanation.
         */
        void print(bool details = false) override { std::cout << to_string(details) << "\n"; };
        std::string to_string(bool details = false) {
            std::stringstream str;
            if (details) {
                str << "TokenList( ";
                for (const auto& token : left) {
                    str << print_details(token.type, token.optype, token.atom);
                }
                str << "| ";
                for (const auto& token : right) {
                    str << print_details(token.type, token.optype, token.atom);
                }
                str << ")\n";
            } else {
                str << "TokenList(" << left.size() << " | " << right.size() << ")\n";
            }
            return str.str();
        }

      private:
        std::string print_details(TokenType type, int optype, AtomGrand* atom) {
            std::stringstream str;
            switch (type) {
            case EMPTY_TOKEN:
                str << "E ";
                break;
            case ATOM_TOKEN:
                str << "A{" << atom->to_string() << "} ";
                break;
            case OPERATOR_TOKEN:
                OperatorBase* op = operators->select(optype);
                str << op->name << " ";
                break;
            }
            return str.str();
        };
    };

} // namespace snt::exs

#endif // EXS_TOKEN_LIST_H
