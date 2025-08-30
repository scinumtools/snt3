#ifndef EXS_TOKEN_H
#define EXS_TOKEN_H

#include "settings.h"

#include <iostream>
#include <string>

namespace exs {

  template <class A>
  class Token {
  public:
    TokenType type;
    int optype;
    A* atom;
    Token() : type(EMPTY_TOKEN), optype(NONE_OPERATOR), atom(nullptr) {}
    Token(TokenType t) : type(t), optype(NONE_OPERATOR), atom(nullptr) {}
    Token(TokenType t, int o) : type(t), optype(o), atom(nullptr) {};
    Token(TokenType t, A* a) : type(t), optype(NONE_OPERATOR), atom(a) {};
    std::string to_string() {
      if (type == EMPTY_TOKEN) {
        return "Token(EMPTY)";
      } else if (type == OPERATOR_TOKEN) {
        return "Token(OPERATOR " + std::to_string(optype) + ")";
      } else {
        return "Token(ATOM " + atom->to_string() + ")";
      }
    }
    void print() {
      std::cout << to_string() << "\n";
    };
  };

} // namespace exs

#endif // EXS_TOKEN_H
