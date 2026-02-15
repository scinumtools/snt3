#ifndef EXS_TOKEN_H
#define EXS_TOKEN_H

#include <snt/exs/settings.h>
#include <snt/exs/atom.h>

#include <iostream>
#include <string>

namespace snt::exs {

  class Token {
  public:
    TokenType type;
    int optype;
    AtomGrand* atom;
    Token() : type(EMPTY_TOKEN), optype(NONE_OPERATOR), atom(nullptr) {}
    Token(TokenType t) : type(t), optype(NONE_OPERATOR), atom(nullptr) {}
    Token(TokenType t, int o) : type(t), optype(o), atom(nullptr) {};
    Token(TokenType t, AtomGrand* a) :
      type(t), optype(NONE_OPERATOR), atom(a) {};
    std::string to_string();
    void print();
  };

} // namespace snt::exs

#endif // EXS_TOKEN_H
