#ifndef EXS_OPERATOR_SUBTRACT_H
#define EXS_OPERATOR_SUBTRACT_H

#include "../token.h"
#include "../token_list.h"
#include "../token_list_base.h"

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorSubtract : public OperatorBase<A, S> {
  public:
    OperatorSubtract(std::string s = "-") : OperatorBase<A, S>("sub", s, SUBTRACT_OPERATOR) {}
    void operate_unary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      if (left.type == EMPTY_TOKEN && right.type == ATOM_TOKEN) {
        right.atom->math_negate();
        tokens->put_right(right);
      } else if (right.optype == ADD_OPERATOR) {
        tokens->put_left(left);
        tokens->put_right(Token<A>(OPERATOR_TOKEN, SUBTRACT_OPERATOR));
      } else if (right.optype == SUBTRACT_OPERATOR) {
        tokens->put_left(left);
        tokens->put_right(Token<A>(OPERATOR_TOKEN, ADD_OPERATOR));
      } else if (left.type != ATOM_TOKEN && right.type == ATOM_TOKEN) {
        tokens->put_left(left);
        right.atom->math_negate();
        tokens->put_right(right);
      } else {
        tokens->put_left(left);
        tokens->put_left(Token<A>(OPERATOR_TOKEN, SUBTRACT_OPERATOR));
        tokens->put_right(right);
      }
    };
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->math_subtract(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_SUBTRACT_H
