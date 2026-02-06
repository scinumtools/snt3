#ifndef NOSTD_CBRT_H
#define NOSTD_CBRT_H

#include "nostd.h"

namespace snt::nostd {

  extern MagnitudeFloat cbrt(const MagnitudeFloat& m);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType cbrt(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude cbrt(const puq::Magnitude& m);
#endif

} // namespace snt::nostd

#endif // NOSTD_CBRT_H
