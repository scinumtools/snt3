#ifndef PUQ_CALCULATOR_ATOM_H
#define PUQ_CALCULATOR_ATOM_H

#include <snt/puq/quantity.h>

namespace snt::puq {

    /** EXS atom that evaluates a PUQ quantity expression. */
    class CalculatorAtom : public exs::AtomBase<CalculatorAtom, Quantity> {
      public:
        CalculatorAtom(const CalculatorAtom& a) = default;
        CalculatorAtom(Quantity v) : AtomBase(v) {};
        static Quantity from_string(std::string& s, exs::BaseSettings* set = nullptr);
        std::string to_string() override;
        /** Add another quantity atom, including its units and uncertainty.
         * @param other Quantity atom to add; dimensions must be compatible.
         */
        void math_add(CalculatorAtom* other) override;
        void math_subtract(CalculatorAtom* other) override;
        void math_negate() override;
        void math_multiply(CalculatorAtom* other) override;
        void math_divide(CalculatorAtom* other) override;
    };

} // namespace snt::puq

#endif // PUQ_CALCULATOR_ATOM_H
