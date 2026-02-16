#ifndef NOSTD_MAX_H
#define NOSTD_MAX_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat max(const puq::MagnitudeFloat& m1, const puq::MagnitudeFloat& m2);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude max(const puq::Magnitude& m1, const puq::Magnitude& m2);
#endif

} // namespace snt::nostd

#endif // NOSTD_MAX_H
