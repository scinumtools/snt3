#ifndef MAT_SETTINGS_H
#define MAT_SETTINGS_H

namespace snt::mat {

  using ElementMap   = std::map<std::string, int>;     ///< Map of elements in a substance and their proportions
  using SubstanceMap = std::map<std::string, double>;  ///< Map of substances in a material and their proportions

  inline constexpr std::string_view MAT_SYMBOL_PLUS = " + ";
  inline constexpr std::string_view MAT_SYMBOL_MULTIPLY = " * ";

  namespace Config {
    inline constexpr bool debug_substance_solver = false;   ///< Show debugging printouts in substance solver
  }
    
}

#endif // MAT_SETTINGS_H
