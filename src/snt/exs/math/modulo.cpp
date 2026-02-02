#include "modulo.h"

namespace snt::exs {

  OperatorModulo::OperatorModulo(std::string s) :
    OperatorBase("mod", s, MODULO_OPERATOR) {
  }
  
  void OperatorModulo::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->math_modulo(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
