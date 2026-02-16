#ifndef MAT_SUBSTANCE_ATOM_H
#define MAT_SUBSTANCE_ATOM_H

#include "../settings.h"
#include "../set.h"

namespace snt::mat {
 
  /**
   * @class SubstanceAtom
   * @brief Substance expression solver
   */
  class SubstanceAtom final: public exs::AtomBase<SubstanceAtom,ElementMap> {
  public:

    /**
     * @brief Copy constructor
     *
     * @param a Reference of an SubstanceAtom object
     */
    SubstanceAtom(SubstanceAtom& a) : AtomBase(a) {};

    /**
     * @brief Create atom from a value
     *
     * @param v ElementMap object
     */
    SubstanceAtom(ElementMap v) : AtomBase(v) {};

    /**
     * @brief Create atom value from a string
     *
     * @param s String representation of a value
     * @param set Pointer to solver settings
     * @return ElementMap object
     */
    static ElementMap from_string(std::string s, exs::BaseSettings* set = nullptr);

    /**
     * @brief Get a string representation of this atom
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
  
}

#endif // MAT_SUBSTANCE_ATOM_H
