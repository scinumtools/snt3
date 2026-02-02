#ifndef EXS_ATOM_H
#define EXS_ATOM_H

#include "settings.h"
#include "atom_base.h"

#include <cmath>
#include <iostream>
#include <math.h>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <variant>

namespace snt::exs {  

  typedef std::variant<double, bool> AtomValueType;  ///< Type of default atom value

  /**
   * @class Atom
   * @brief Default atom class
   */
  class Atom final: public AtomBase<Atom,AtomValueType> {
  public:

    /**
     * @brief Copy constructor of an atom
     *
     * @param a Reference to an atom from which we copy the values
     */
    Atom(const Atom& a) : AtomBase(a) {};

    /**
     * @brief Initialise an empty vector of a specific type
     *
     * @param v Atom value type
     */
    Atom(AtomValueType v) : AtomBase(v) {};

    /**
     * @brief Create and atom object from a string
     *
     * @param s String corresponding to an atom
     * @param set Pointer to solver settings
     */
    static AtomValueType from_string(std::string s, BaseSettings* set = nullptr);
     
    /**
     * @copydoc AtomGrand::to_string
     */
    std::string to_string() override;
    
    // Math operations

    /**
     * @copydoc AtomGrand::math_add
     */
    void math_add(Atom* other) override;

    /**
     * @copydoc AtomGrand::math_subtract
     */
    void math_subtract(Atom* other) override;

    /**
     * @copydoc AtomGrand::math_negate
     */
    void math_negate() override;

    /**
     * @copydoc AtomGrand::math_multiply
     */
    void math_multiply(Atom* other) override;

    /**
     * @copydoc AtomGrand::math_divide
     */
    void math_divide(Atom* other) override;

    /**
     * @copydoc AtomGrand::math_power
     */
    void math_power(Atom* other) override;

    /**
     * @copydoc AtomGrand::math_modulo
     */
    void math_modulo(Atom* other) override;

    // Argument math operators

    /**
     * @copydoc AtomGrand::math_exponent
     */
    void math_exponent() override;

    /**
     * @copydoc AtomGrand::math_logarithm
     */
    void math_logarithm() override;

    /**
     * @copydoc AtomGrand::math_logarithm_10
     */
    void math_logarithm_10() override;

    /**
     * @copydoc AtomGrand::math_logarithm_base
     */
    void math_logarithm_base(Atom* other) override;

    /**
     * @copydoc AtomGrand::math_power_base
     */
    void math_power_base(Atom* other) override;

    /**
     * @copydoc AtomGrand::math_square_root
     */
    void math_square_root() override;

    /**
     * @copydoc AtomGrand::math_cubic_root
     */
    void math_cubic_root() override;

    /**
     * @copydoc AtomGrand::math_sinus
     */
    void math_sinus() override;

    /**
     * @copydoc AtomGrand::math_cosinus
     */
    void math_cosinus() override;

    /**
     * @copydoc AtomGrand::math_tangens
     */
    void math_tangens() override;

    // Comparison operations
    
    /**
     * @copydoc AtomGrand::comparison_equal
     */
    void comparison_equal(Atom* other) override;
    
    /**
     * @copydoc AtomGrand::comparison_not_equal
     */
    void comparison_not_equal(Atom* other) override;
    
    /**
     * @copydoc AtomGrand::comparison_less_equal
     */
    void comparison_less_equal(Atom* other) override;
    
    /**
     * @copydoc AtomGrand::comparison_greater_equal
     */
    void comparison_greater_equal(Atom* other) override;
    
    /**
     * @copydoc AtomGrand::comparison_less
     */
    void comparison_less(Atom* other) override;
    
    /**
     * @copydoc AtomGrand::comparison_greater
     */
    void comparison_greater(Atom* other) override;

    // Logical operations
    
    /**
     * @copydoc AtomGrand::logical_not
     */
    void logical_not() override;

    /**
     * @copydoc AtomGrand::logical_and
     */
    void logical_and(Atom* other) override;

    /**
     * @copydoc AtomGrand::logical_or
     */
    void logical_or(Atom* other) override;

    // Conditional operator
    
    /**
     * @copydoc AtomGrand::condition
     */
    void condition(Atom* option1, Atom* option2) override;
  };

} // namespace snt::exs

#endif // EXS_ATOM_H
