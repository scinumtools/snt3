#ifndef MAT_SUBSTANCE_ATOM_H
#define MAT_SUBSTANCE_ATOM_H

#include <snt/mat/set.h>
#include <snt/mat/settings.h>

namespace snt::mat {

    /**
     * Substance expression solver
     */
    class SubstanceAtom final : public exs::AtomBase<SubstanceAtom, ElementMap> {
      public:
        /**
         * Copy constructor
         *
         * @param a Reference of an SubstanceAtom object
         */
        SubstanceAtom(SubstanceAtom& a) : AtomBase(a) {};

        /**
         * Create atom from a value
         *
         * @param v ElementMap object
         */
        SubstanceAtom(ElementMap v) : AtomBase(v) {};

        /**
         * Create atom value from a string
         *
         * @param s String representation of a value
         * @param set Pointer to solver settings
         * @return ElementMap object
         */
        static ElementMap from_string(std::string s, exs::BaseSettings* set = nullptr);

        /**
         * Get a string representation of this atom
         *
         * @return String representation of this atom
         */
        std::string to_string() override;

        /**
         * @copydoc exs::AtomBase::math_add
         */
        void math_add(SubstanceAtom* other) override;

        /**
         * @copydoc exs::AtomBase::math_multiply
         */
        void math_multiply(SubstanceAtom* other) override;
    };

} // namespace snt::mat

#endif // MAT_SUBSTANCE_ATOM_H
