#include <snt/exs/expression.h>

#include <string>

namespace snt::exs {

  Expression::Expression(std::string expression) {
    left = "";
    right = expression;
    expr = expression;
  };
  
  void Expression::shift(int nchar) {
    left += right.substr(0, nchar);
    right = right.substr(nchar, right.length());
  };
  
  void Expression::remove(std::string string) {
    right = right.substr(string.length(), right.length());
  };
  
  std::string Expression::pop_left() {
    std::string dummy = trim(left);
    left = "";
    return dummy;
  };
  
  std::string Expression::to_string() {
    std::stringstream ss;
    ss << "Expression(L'" << left << "' R'" << right << "' E'" << expr << "')";
    return ss.str();
  }
  
  void Expression::print() {
    std::cout << to_string() << std::endl;
  };
  
}
