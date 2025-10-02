#ifndef NOSTD_CBRT_H
#define NOSTD_CBRT_H

#include "nostd.h"

namespace snt::nostd {

  extern MAGNITUDE_PRECISION cbrt(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_ARRAYS)
  extern Array cbrt(const Array& a);
#elif defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType cbrt(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude cbrt(const puq::Magnitude& m);
#endif

} // namespace snt::nostd

#endif // NOSTD_CBRT_H
