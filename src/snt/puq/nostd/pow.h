#ifndef NOSTD_POW_H
#define NOSTD_POW_H

namespace snt::nostd {

  extern MAGNITUDE_PRECISION pow(const MAGNITUDE_PRECISION& m, const EXPONENT_FLOAT_PRECISION& e);
#if defined(MAGNITUDE_ARRAYS)
  extern Array pow(const Array& a, const EXPONENT_FLOAT_PRECISION& e);
  extern Array pow(const Array& a, const Array& e);
#elif defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, const EXPONENT_FLOAT_PRECISION& e);
  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude pow(const puq::Magnitude& m, const EXPONENT_FLOAT_PRECISION& e);
  extern puq::Magnitude pow(const puq::Magnitude& m, const puq::Magnitude& e);
#endif

} // namespace snt::nostd

#endif // NOSTD_POW_H
