#ifndef PUQ_CALCULATOR_H
#define PUQ_CALCULATOR_H

#include <memory>
#include <snt/puq/calc/calculator_atom.h>

namespace snt::puq {

    class Calculator {
      public:
        static exs::Solver<CalculatorAtom> solver;
        Calculator();
        CalculatorAtom eval(const std::string& expression);
    };

} // namespace snt::puq

#endif // PUQ_CALCULATOR_H
