#ifndef DIP_NUMERICAL_ATOM_H
#define DIP_NUMERICAL_ATOM_H

#include <snt/dip/environment.h>
#include <snt/puq/quantity.h>

namespace snt::dip {

    struct NumericalSettings : exs::BaseSettings {
        Environment* env;
    };

    class NumericalAtom : public exs::AtomBase<NumericalAtom, ValueNodeData> {
      public:
        // Constructor from unique_ptr
        NumericalAtom(ValueNodeData b) : AtomBase({std::move(b.value), std::move(b.units)}) {};
        // Deep copy constructor
        NumericalAtom(const NumericalAtom& a) : AtomBase({a.value.value->clone(), a.value.units}) {};
        NumericalAtom& operator=(const NumericalAtom& a);
        // Move constructor
        NumericalAtom(NumericalAtom&& a) noexcept = default;
        NumericalAtom& operator=(NumericalAtom&& a) noexcept = default;

        static ValueNodeData from_string(const std::string& s, exs::BaseSettings* settings);
        std::string to_string() override;
        void math_sinus() override;
        void math_cosinus() override;
        void math_tangens() override;
        void math_cubic_root() override;
        void math_square_root() override;
        // void math_power_base(NumericalAtom* other) override;
        // void math_logarithm_base(NumericalAtom* other) override;
        void math_logarithm_10() override;
        void math_logarithm() override;
        void math_exponent() override;
        void math_power(NumericalAtom* other) override;
        void math_multiply(NumericalAtom* other) override;
        void math_divide(NumericalAtom* other) override;
        void math_add(NumericalAtom* other) override;
        void math_subtract(NumericalAtom* other) override;
        void math_negate() override;
    };

} // namespace snt::dip

#endif // DIP_NUMERICAL_ATOM_H
