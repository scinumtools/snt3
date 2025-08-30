#ifndef EXS_OPERATOR_ADD_H
#define EXS_OPERATOR_ADD_H

#include "../token.h"
#include "../token_list_base.h"
#include "../token_list.h"

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorAdd : public OperatorBase<A, S> {
  public:
    OperatorAdd(std::string s = "+") : OperatorBase<A, S>("add", s, ADD_OPERATOR) {}
    void operate_unary(TokenListBase<A>* tokens) override {
      // std::cout << "operate_unary" << std::endl;
      // tokens->print(true);
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      if (left.type == EMPTY_TOKEN && right.type == ATOM_TOKEN) {
        // std::cout << "a " << left.to_string() << " " << right.to_string() << std::endl;
        tokens->put_left(right);
      } else if (right.optype == ADD_OPERATOR) {
        // std::cout << "b " << left.to_string() << " " << right.to_string() << std::endl;
        tokens->put_left(left);
        tokens->put_right(right);
      } else if (right.optype == SUBTRACT_OPERATOR) {
        // std::cout << "c" << std::endl;
        tokens->put_left(left);
        tokens->put_right(right);
      } else if (left.type != ATOM_TOKEN && right.type == ATOM_TOKEN) {
        // std::cout << "d" << std::endl;
        tokens->put_left(left);
        tokens->put_right(right);
      } else {
        // std::cout << "e" << std::endl;
        tokens->put_left(left);
        tokens->put_left(Token<A>(OPERATOR_TOKEN, ADD_OPERATOR));
        tokens->put_right(right);
      }
    };
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->math_add(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_ADD_H
