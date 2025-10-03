#ifndef NOSTD_MAX_H
#define NOSTD_MAX_H

#include "nostd.h"

namespace snt::nostd {

  extern MAGNITUDE_PRECISION max(const MAGNITUDE_PRECISION& m1, const MAGNITUDE_PRECISION& m2);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude max(const puq::Magnitude& m1, const puq::Magnitude& m2);
#endif

} // namespace snt::nostd

#endif // NOSTD_MAX_H
