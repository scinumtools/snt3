#ifndef EXS_ATOM_GRAND_H
#define EXS_ATOM_GRAND_H

#include <string>

namespace snt::exs {

  /**
   * @class AtomGrand
   * @brief Virtual base class actually used in the solver
   */
  class AtomGrand {
  public:
    /**
     * @brief Default class destructor
     */
    virtual ~AtomGrand() = default;

    /**
     * @brief Get a string representation of an atom
     */
    virtual std::string to_string() = 0;

    /**
     * @brief Print out atoms representation
     */
    void print();

    // Math operations

    /**
     * @brief Add this value to another value
     *
     * @param other Pointer to another atom
     */
    virtual void math_add(AtomGrand* other) {};

    /**
     * @brief Subtract another value from this value
     *
     * @param other Pointer to another atom
     */
    virtual void math_subtract(AtomGrand* other) {};

    /**
     * @brief Negate value of this value
     */
    virtual void math_negate() {};

    /**
     * @brief Multiply this value with another value
     *
     * @param other Pointer to another atom
     */
    virtual void math_multiply(AtomGrand* other) {};

    /**
     * @brief Divide this value with another value
     *
     * @param other Pointer to another atom
     */
    virtual void math_divide(AtomGrand* other) {};

    /**
     * @brief Raise this values to a given power
     *
     * @param other Pointer to another atom
     */
    virtual void math_power(AtomGrand* other) {};

    /**
     * @brief Perform modulo on this atom
     *
     * @param other Pointer to another atom
     */
    virtual void math_modulo(AtomGrand* other) {};

    // Argument math operators
    
    /**
     * @brief Get an exponential value of this value
     */
    virtual void math_exponent() {};
    
    /**
     * @brief Get a natural logarithm of this value
     */
    virtual void math_logarithm() {};
    
    /**
     * @brief Get a decadic logarithm of this value
     */
    virtual void math_logarithm_10() {};
    
    /**
     * @brief Get a logarithm with arbitrary base of this value
     *
     * @param other Pointer to another atom
     */
    virtual void math_logarithm_base(AtomGrand* other) {};
    
    /**
     * @brief Get power with arbitrary base of this value
     *
     * @param other Pointer to another atom
     */
    virtual void math_power_base(AtomGrand* other) {};
    
    /**
     * @brief Get square root value of this value
     */
    virtual void math_square_root() {};
    
    /**
     * @brief Get cubic root of this value
     */
    virtual void math_cubic_root() {};
    
    /**
     * @brief Get sinus of this value
     */
    virtual void math_sinus() {};
    
    /**
     * @brief Get cosinus of this value
     */
    virtual void math_cosinus() {};
    
    /**
     * @brief Get tangens value of this value
     */
    virtual void math_tangens() {};

    // Comparison operations
        
    /**
     * @brief Compare equality of this and other value
     *
     * @param other Pointer to another atom
     */
    virtual void comparison_equal(AtomGrand* other) {};
        
    /**
     * @brief Compare difference of ths and other value
     *
     * @param other Pointer to another atom
     */
    virtual void comparison_not_equal(AtomGrand* other) {};
        
    /**
     * @brief Compare if this value is less or equal than other
     *
     * @param other Pointer to another atom
     */
    virtual void comparison_less_equal(AtomGrand* other) {};
        
    /**
     * @brief Compare if this value is greater or equal than other
     *
     * @param other Pointer to another atom
     */
    virtual void comparison_greater_equal(AtomGrand* other) {};
        
    /**
     * @brief Compare if this value is less than other
     *
     * @param other Pointer to another atom
     */
    virtual void comparison_less(AtomGrand* other) {};
        
    /**
     * @brief Compare if this value is greater than other
     *
     * @param other Pointer to another atom
     */
    virtual void comparison_greater(AtomGrand* other) {};

    // Logical operations
    
        
    /**
     * @brief Get logical negation of this value
     */
    virtual void logical_not() {};
        
    /**
     * @brief Get logical and of this and other value
     *
     * @param other Pointer to another atom
     */
    virtual void logical_and(AtomGrand* other) {};
        
    /**
     * @brief Get logical or of this and other value
     *
     * @param other Pointer to another atom
     */
    virtual void logical_or(AtomGrand* other) {};

    // Conditional operator    
        
    /**
     * @brief Decide between an option1 and option2 based of this value
     *
     * @param option1 Pointer to another atom
     * @param option2 Pointer to another atom
     */
    virtual void condition(AtomGrand* option1, AtomGrand* option2) {};
  };

}

#endif // EXS_ATOM_GRAND_H
