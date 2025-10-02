#ifndef NOSTD_LOG_H
#define NOSTD_LOG_H

#include "nostd.h"

namespace snt::nostd {

  extern MAGNITUDE_PRECISION log(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_ARRAYS)
  extern Array log(const Array& a);
#elif defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType log(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude log(const puq::Magnitude& m);
#endif

} // namespace snt::nostd

#endif // NOSTD_LOG_H
