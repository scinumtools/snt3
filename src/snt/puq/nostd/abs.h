#ifndef NOSTD_ABS_H
#define NOSTD_ABS_H

#include "nostd.h"

namespace snt::nostd {

  extern MAGNITUDE_PRECISION abs(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType abs(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude abs(const puq::Magnitude& m);
#endif

} // namespace snt::nostd

#endif // NOSTD_ABS_H
