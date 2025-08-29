#ifndef PUQ_NOSTD_H
#define PUQ_NOSTD_H

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
// #include <omp.h>

#include "../array.h"
#include "../magnitude.h"
#include "../settings.h"
#include "../value/value.h"

namespace nostd {

    /*
     *  stof, stod, stold -> to_number
     */

    inline MAGNITUDE_PRECISION to_number(const std::string& value) {
      if (typeid(MAGNITUDE_PRECISION) == typeid(float))
        return std::stof(value);
      else if (typeid(MAGNITUDE_PRECISION) == typeid(double))
        return std::stod(value);
      else if (typeid(MAGNITUDE_PRECISION) == typeid(long double))
        return std::stold(value);
      else
        throw std::range_error("Unsupported magnitude precision.");
    }

    /*
     *  to_string
     */

    extern std::string to_string(const MAGNITUDE_PRECISION& value, int precision = std::cout.precision());
#ifdef EXPONENT_FRACTIONS
    extern std::string to_string(const puq::Exponent& value, const puq::UnitFormat& format = puq::UnitFormat());
#endif
#if defined(MAGNITUDE_ARRAYS)
    extern std::string to_string(const Array& value, int precision = std::cout.precision());
#elif defined(MAGNITUDE_VALUES)
    extern std::string to_string(val::BaseValue::PointerType value, int precision = std::cout.precision());
#endif
#ifdef MAGNITUDE_ERRORS
    extern std::string to_string(const puq::Magnitude& value, int precision = std::cout.precision());
#endif
    extern std::string to_string(const puq::BaseUnits& value, int precision = std::cout.precision());
    extern std::string to_string(const puq::Dimensions& value, const puq::UnitFormat& format = puq::UnitFormat());
    extern std::string to_string(const bool& use_prefixes, const puq::AllowedPrefixes& value);
#if defined(MAGNITUDE_ARRAYS)
    extern std::string to_string(const ArrayShape& shape);
#endif

    /*
     *  exp
     */

    extern MAGNITUDE_PRECISION exp(const MAGNITUDE_PRECISION& e);
#if defined(MAGNITUDE_ARRAYS)
    extern Array exp(const Array& e);
#elif defined(MAGNITUDE_VALUES)
    extern val::BaseValue::PointerType exp(val::BaseValue::PointerType e);
#endif
#ifdef MAGNITUDE_ERRORS
    extern puq::Magnitude exp(const puq::Magnitude& e);
#endif
    extern puq::UnitValue exp(const puq::UnitValue& uv);

    /*
     * log
     */

    extern MAGNITUDE_PRECISION log(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_ARRAYS)
    extern Array log(const Array& a);
#elif defined(MAGNITUDE_VALUESS)
    extern val::BaseValue::PointerType log(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
    extern puq::Magnitude log(const puq::Magnitude& m);
#endif

    /*
     * log10
     */

    extern MAGNITUDE_PRECISION log10(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_ARRAYS)
    extern Array log10(const Array& a);
#elif defined(MAGNITUDE_VALUES)
    extern val::BaseValue::PointerType log10(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
    extern puq::Magnitude log10(const puq::Magnitude& m);
#endif

    /*
     * sqrt
     */

    extern MAGNITUDE_PRECISION sqrt(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_ARRAYS)
    extern Array sqrt(const Array& a);
#elif defined(MAGNITUDE_VALUES)
    extern val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
    extern puq::Magnitude sqrt(const puq::Magnitude& m);
#endif

    /*
     * cbrt
     */

    extern MAGNITUDE_PRECISION cbrt(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_ARRAYS)
    extern Array cbrt(const Array& a);
#elif defined(MAGNITUDE_VALUES)
    extern val::BaseValue::PointerType cbrt(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
    extern puq::Magnitude cbrt(const puq::Magnitude& m);
#endif

    /*
     *  pow
     */

    extern MAGNITUDE_PRECISION pow(const MAGNITUDE_PRECISION& m, const EXPONENT_REAL_PRECISION& e);
#if defined(MAGNITUDE_ARRAYS)
    extern Array pow(const Array& a, const EXPONENT_REAL_PRECISION& e);
    extern Array pow(const Array& a, const Array& e);
#elif defined(MAGNITUDE_VALUES)
    extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, const EXPONENT_REAL_PRECISION& e);
    extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e);
#endif
#ifdef MAGNITUDE_ERRORS
    extern puq::Magnitude pow(const puq::Magnitude& m, const EXPONENT_REAL_PRECISION& e);
    extern puq::Magnitude pow(const puq::Magnitude& m, const puq::Magnitude& e);
#endif

    /*
     * floor
     */

    extern MAGNITUDE_PRECISION floor(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_ARRAYS)
    extern Array floor(const Array& a);
#elif defined(MAGNITUDE_VALUES)
    extern val::BaseValue::PointerType floor(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
    extern puq::Magnitude floor(const puq::Magnitude& m);
#endif

    /*
     * abs
     */

    extern MAGNITUDE_PRECISION abs(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_ARRAYS)
    extern Array abs(const Array& a);
#elif defined(MAGNITUDE_VALUES)
    extern val::BaseValue::PointerType abs(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
    extern puq::Magnitude abs(const puq::Magnitude& m);
#endif

    /*
     * max
     */

    extern MAGNITUDE_PRECISION max(const MAGNITUDE_PRECISION& m1, const MAGNITUDE_PRECISION& m2);
#if defined(MAGNITUDE_ARRAYS)
    extern Array max(const Array& a1, const Array& a2);
#elif defined(MAGNITUDE_VALUES)
    extern val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2);
#endif
#ifdef MAGNITUDE_ERRORS
    extern puq::Magnitude max(const puq::Magnitude& m1, const puq::Magnitude& m2);
#endif

} // namespace nostd

#endif // PUQ_NOSTD_H
