#ifndef EXS_EXPRESSION_H
#define EXS_EXPRESSION_H

#include <snt/exs/settings.h>

#include <iostream>
#include <sstream>

namespace snt::exs {

  class Expression {
  public:
    std::string left;  // left side
    std::string right; // right side
    std::string expr;  // original expression
    Expression(std::string expression);
    void shift(int nchar = 1);
    void remove(std::string string);
    std::string pop_left();
    std::string to_string();
    void print();
  };

} // namespace snt::exs

#endif // EXS_EXPRESSION_H
