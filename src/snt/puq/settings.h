#ifndef PUQ_SETTINGS_H
#define PUQ_SETTINGS_H

#ifdef CONFIG_FILE
#include CONFIG_FILE
#else
#include "config.h"
#endif

/*
 *  Debugging tools
 */
// #define DEBUG_UNIT_SOLVER
// #define DEBUG_CALCULATOR
// #define DEBUG_CONVERTER

/*
 *  General settings
 */
#define NUM_BASEDIM 8
#define SYMBOL_PLUS "+"
#define SYMBOL_MINUS "-"
#define SYMBOL_MINUS2 "\u2212"
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

/*
 *  Module specific settings
 */
#ifdef EXPONENT_FRACTIONS
#define SYMBOL_FRACTION ":"
#define SYMBOL_FRACTION2 "\u141F"
#endif
#ifdef PREPROCESS_SYSTEM
#define SYMBOL_SYSTEM "_"
#endif

/*
 *  Type settings
 */
#if defined(MAGNITUDE_ERRORS)
#define MAGNITUDE_TYPE Magnitude
#if defined(MAGNITUDE_VALUES)
#define MAGNITUDE_VALUE val::BaseValue::PointerType
#else
#define MAGNITUDE_VALUE MAGNITUDE_PRECISION
#endif
#elif defined(MAGNITUDE_VALUES)
#define MAGNITUDE_TYPE val::BaseValue::PointerType
#else
#define MAGNITUDE_TYPE MAGNITUDE_PRECISION
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
#define EXPONENT_TYPE EXPONENT_INT_PRECISION
#endif

#endif // PUQ_SETTINGS_H
