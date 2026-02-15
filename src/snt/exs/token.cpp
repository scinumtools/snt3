#include <snt/exs/token.h>

namespace snt::exs {

  std::string Token::to_string() {
    if (type == EMPTY_TOKEN) {
      return "Token(EMPTY)";
    } else if (type == OPERATOR_TOKEN) {
      return "Token(OPERATOR " + std::to_string(optype) + ")";
    } else {
      return "Token(ATOM " + atom->to_string() + ")";
    }
  }
  
  void Token::print() {
    std::cout << to_string() << "\n";
  };

}
