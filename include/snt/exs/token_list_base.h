#ifndef EXS_TOKEN_LIST_BASE_H
#define EXS_TOKEN_LIST_BASE_H

#include <snt/exs/token.h>

namespace snt::exs {

  class TokenListBase {
  public:
    virtual Token get_left() = 0;
    virtual Token get_right() = 0;
    virtual void put_left(Token t) = 0;
    virtual void put_right(Token t) = 0;
    virtual void print(bool details = false) = 0;
  };

} // namespace snt::exs

#endif // EXS_TOKEN_LIST_BASE_H
