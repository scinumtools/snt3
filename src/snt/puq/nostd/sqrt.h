#ifndef NOSTD_SQRT_H
#define NOSTD_SQRT_H

#include "nostd.h"

namespace snt::nostd {

  extern MAGNITUDE_PRECISION sqrt(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude sqrt(const puq::Magnitude& m);
#endif

} // namespace snt::nostd

#endif // NOSTD_SQRT_H
