#ifndef MAT_SET_H
#define MAT_SET_H

#include "part.h"

#include <string>
#include <map>

namespace snt::mat {

  /**
   * @class Set
   * @brief A set is a material or object made up of two or more distinct members that remain physically separate and identifiable within the final structure
   *
   * @tparam SOLVER Type of an expression solver used by a set
   * @tparam MEMBER Type of set members
   * @tparam PROPORTION Proportion data type
   */
  template <typename SOLVER, typename MEMBER, typename PROPORTION>
  class Set {
  public:

    using PartMap = std::map<std::string, PROPORTION>;  ///< Map of parts and their proportions

    std::map<std::string, Part<PROPORTION>> parts;   ///< Map of parts
    std::string expression;                          ///< String representation of a set
    bool natural;                                    ///< true if set is using natural abundances
    
    /**
     * @brief Set class constructor
     *
     * @param expr Set string representation
     * @param nat Indicate if substance atoms should use natural abundance
     */
    Set(const std::string& expr, bool nat = true):
      expression(expr), natural(nat) {

      SOLVER solver;      
    };

    /**
     * @brief Set class constructor
     *
     * @param comp A map of parts with their proportions
     * @param nat Indicate if substance atoms should use natural abundance
     * @param expr Set string representation     
     */
    Set(const PartMap& comp, bool nat = true, const std::string& expr = ""):
      expression(expr), natural(nat) {

      for (const auto& [pexpr, pprop]: comp) {
      }
    };
    
  };

} // namespace snt::mat

#endif // MAT_SET_H
