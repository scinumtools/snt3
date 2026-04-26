#ifndef EXS_EXPRESSION_H
#define EXS_EXPRESSION_H

#include <iostream>
#include <snt/exs/settings.h>
#include <sstream>

namespace snt::exs {

  class Expression {
  public:
    std::string left;  // left side
    std::string right; // right side
    std::string expr;  // original expression
    Expression(const std::string& expression);
    void shift(int nchar = 1);
    void remove(const std::string& string);
    std::string pop_left();
    std::string to_string();
    void print();
  };

} // namespace snt::exs

#endif // EXS_EXPRESSION_H
