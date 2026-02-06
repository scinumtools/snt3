#ifndef NOSTD_POW_H
#define NOSTD_POW_H

#include "nostd.h"

namespace snt::nostd {

  extern MagnitudeFloat pow(const MagnitudeFloat& m, const ExponentFloat& e);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, const ExponentFloat& e);
  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude pow(const puq::Magnitude& m, const ExponentFloat& e);
  extern puq::Magnitude pow(const puq::Magnitude& m, const puq::Magnitude& e);
#endif

} // namespace snt::nostd

#endif // NOSTD_POW_H
