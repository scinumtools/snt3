#ifndef PUQ_SETTINGS_H
#define PUQ_SETTINGS_H

/*
 *  Debugging tools
 */
// #define DEBUG_UNIT_SOLVER
// #define DEBUG_CALCULATOR
// #define DEBUG_CONVERTER

/*
 *  Numerical precision
 */
using MagnitudeFloat = double;
using ExponentInt = int;
using ExponentFloat = double;

/*
 *  Modules
 */

#define MAGNITUDE_ERRORS
#if (ENABLE_PUQ_MAGNITUDE_VALUE)
#define MAGNITUDE_VALUES
#endif
#define EXPONENT_FRACTIONS

namespace snt::puq::Config {
  inline constexpr int num_basedim = 8;                 ///< Number of base dimensions
  inline constexpr bool use_units_temperature = true;   ///< Use temperature units
  inline constexpr bool use_units_logarithmic = true;   ///< Use logarithmic units
  inline constexpr bool use_system_cgs = true;          ///< Use CGS systems of units
  inline constexpr bool use_system_nus = true;          ///< Use natural systems of units
  inline constexpr bool use_system_eus = true;          ///< Use english system of units
  inline constexpr bool preprocess_system = true;       ///< Preprocess system prefix in quantity expressions 
  inline constexpr bool preprocess_symbols = true;      ///< Preprocess special symbols in quantity expressions
}
#define UNITS_TEMPERATURES
#define UNITS_LOGARITHMIC
#define UNIT_SYSTEM_CGS
#define UNIT_SYSTEM_NUS
#define UNIT_SYSTEM_EUS

/*
 *  General settings
 */
#define SYMBOL_MULTIPLY "*"
#define SYMBOL_MULTIPLY2 "\u22C5"
#define SYMBOL_DIVIDE "/"
#define SYMBOL_POWER "^"
#define SYMBOL_EXPONENT 'e'
#define SYMBOL_EXPONENT2 "\u00D710" // x10
#define SYMBOL_QUANTITY_START '<'
#define SYMBOL_QUANTITY_END ">"
#define SYMBOL_SIFACTOR_START '|'
#define SYMBOL_SIFACTOR_END "|"

namespace snt::puq::Symbols {
  inline constexpr std::string_view plus = "+";                     ///< Symbol plus
  inline constexpr std::string_view minus = "-";                    ///< Symbol minus (e.g. m-2)
  inline constexpr std::string_view minus2 = "\u2212";              ///< Symbol minus (e.g. −34e3)
  inline constexpr std::string_view fraction_separator = ":";       ///< Separator of nominator and denominator in an exponent fraction (e.g. m3:2)
  inline constexpr std::string_view fraction_separator2 = "\u141F"; ///< Separator of nominator and denominator in an exponent fraction (e.g. m³ᐟ²)
  inline constexpr std::string_view system_separator = "_";         ///< Separator of the unit system and quantity in expressions (e.g. SI_m)
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
#ifdef UNIT_SYSTEM_CGS
static_assert(false, "Unit system CGS cannot be used without fractional exponents! Please use EXPONENT_FRACTIONS flag.");
#endif
#ifdef UNIT_SYSTEM_NUS
static_assert(false, "Natural units cannot be used without fractional exponents! Please use EXPONENT_FRACTIONS flag.");
#endif
#define EXPONENT_TYPE ExponentInt
#endif

#endif // PUQ_SETTINGS_H
