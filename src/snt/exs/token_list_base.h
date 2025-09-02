#ifndef EXS_TOKEN_LIST_BASE_H
#define EXS_TOKEN_LIST_BASE_H

#include "token.h"

namespace snt::exs {

  template <class A>
  class TokenListBase {
  public:
    virtual Token<A> get_left() = 0;
    virtual Token<A> get_right() = 0;
    virtual void put_left(Token<A> t) = 0;
    virtual void put_right(Token<A> t) = 0;
    virtual void print(bool details = false) = 0;
  };

} // namespace snt::exs

#endif // EXS_TOKEN_LIST_BASE_H
