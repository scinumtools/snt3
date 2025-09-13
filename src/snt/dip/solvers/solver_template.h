#ifndef DIP_SOLVER_TEMPLATE_H
#define DIP_SOLVER_TEMPLATE_H

#include "../../exs.h"
#include "../../val.h"

#include <string>

namespace snt::dip {

  class TemplateSolver {
    Environment* environment;
  public:
    TemplateSolver(Environment& env);
    std::string eval(std::string expression);
  };

}

#endif // DIP_SOLVER_TEMPLATE_H
