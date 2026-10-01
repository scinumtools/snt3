#ifndef EXS_TOKEN_H
#define EXS_TOKEN_H

#include <iostream>
#include <optional>
#include <snt/exs/atom.h>
#include <snt/exs/settings.h>
#include <string>
#include <vector>

namespace snt::exs {

    /** Lexical token produced by the EXS expression parser. */
    class Token {
      public:
        TokenType type;
        int optype;
        AtomGrand* atom;
        std::optional<std::size_t> composition_node; ///< Set only when composition recording is requested.
        std::size_t group_count = 0;
        std::vector<int> prefix_operators; ///< Original prefix signs if EXS folds adjacent signs.
        Token() : type(EMPTY_TOKEN), optype(NONE_OPERATOR), atom(nullptr) {}
        Token(TokenType t) : type(t), optype(NONE_OPERATOR), atom(nullptr) {}
        Token(TokenType t, int o) : type(t), optype(o), atom(nullptr) {};
        Token(TokenType t, AtomGrand* a) : type(t), optype(NONE_OPERATOR), atom(a) {};
        std::string to_string();
        void print();
    };

} // namespace snt::exs

#endif // EXS_TOKEN_H
