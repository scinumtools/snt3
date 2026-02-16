#ifndef PUQ_SETTINGS_H
#define PUQ_SETTINGS_H

#include <string>

namespace snt::puq {

  using MagnitudeFloat = double;  ///< Numerical precision of a magnitude
  using ExponentInt = int;        ///< Numerical precision of an integer exponent
  using ExponentFloat = double;   ///< Numerical precision of a floating-point exponent

  /*
   *  Modules
   */

#define MAGNITUDE_ERRORS
#if (ENABLE_PUQ_MAGNITUDE_VALUE)
#define MAGNITUDE_VALUES
#endif
#define EXPONENT_FRACTIONS

  /**
   * @namespace Config
   * @brief Configuration constants
   */
  namespace Config {
    // default settings
    inline constexpr int num_basedim = 8;                 ///< Number of base dimensions
    // special units
    inline constexpr bool use_units_temperature = true;   ///< Use temperature units
    inline constexpr bool use_units_logarithmic = true;   ///< Use logarithmic units
    // system of units
    inline constexpr bool use_system_cgs = true;          ///< Use CGS systems of units
    inline constexpr bool use_system_nus = true;          ///< Use natural systems of units
    inline constexpr bool use_system_eus = true;          ///< Use english system of units
    // preprocessing
    inline constexpr bool preprocess_system = true;       ///< Preprocess system prefix in quantity expressions 
    inline constexpr bool preprocess_symbols = true;      ///< Preprocess special symbols in quantity expressions
    // debugging
    inline constexpr bool debug_unit_solver = false;      ///< Switch on debugging diagnostics for UnitSolver
    inline constexpr bool debug_calculator = false;       ///< Switch on debugging diagnostics for Calculator
    inline constexpr bool debug_converter = false;        ///< Switch on debugging diagnostics for Converter
  }

  /**
   * @namespace Symbols
   * @brief Symbol constants
   */
  namespace Symbols {
    // quantity expression symbols
    inline constexpr std::string_view plus = "+";                     ///< Symbol plus +
    inline constexpr std::string_view minus = "-";                    ///< Symbol minus -
    inline constexpr std::string_view minus2 = "\u2212";              ///< Symbol minus −
    inline constexpr std::string_view multiply = "*";                 ///< Symbol multiplication *
    inline constexpr std::string_view multiply2 = "\u22C5";           ///< Symbol multiplication ⋅
    inline constexpr std::string_view divide = "/";                   ///< Symbol division /
    inline constexpr std::string_view power = "^";                    ///< Symbol power ^
    inline constexpr std::string_view exponent = "e";                 ///< Symbol exponent e
    inline constexpr std::string_view exponent2 = "\u00D710";         ///< Symbol exponent ×10
    inline constexpr std::string_view fraction_separator = ":";       ///< Separator in an exponent fraction (e.g. m3:2)
    inline constexpr std::string_view system_separator = "_";         ///< Separator of an unit system in expressions (e.g. SI_m)
    // special unit symbols
    inline constexpr std::string_view quantity_start = "<";           ///< Starting symbol of a physical quantity unit (e.g. <a>)
    inline constexpr std::string_view quantity_end = ">";             ///< Ending symbol of a physical quantity unit
    inline constexpr std::string_view si_factor_start = "|";          ///< Starting symbol of an SI conversion factor (e.g. |A|)
    inline constexpr std::string_view si_factor_end = "|";            ///< Ending symbol of an SI conversion factor
  }

  /*
   *  Type settings
   */
#if defined(MAGNITUDE_ERRORS)
#define MAGNITUDE_TYPE Magnitude
#if defined(MAGNITUDE_VALUES)
#define MAGNITUDE_VALUE val::BaseValue::PointerType
#else
#define MAGNITUDE_VALUE MagnitudeFloat
#endif
#elif defined(MAGNITUDE_VALUES)
#define MAGNITUDE_TYPE val::BaseValue::PointerType
#else
#define MAGNITUDE_TYPE MagnitudeFloat
#endif

#ifdef EXPONENT_FRACTIONS
#define EXPONENT_TYPE Exponent
#else
  static_assert(Config::use_system_cgs, "Unit system CGS cannot be used without fractional exponents! Please use EXPONENT_FRACTIONS flag.");
  static_assert(Config::use_system_nus, "Natural units cannot be used without fractional exponents! Please use EXPONENT_FRACTIONS flag.");
#define EXPONENT_TYPE ExponentInt
#endif


}
  
#endif // PUQ_SETTINGS_H
