#ifndef NOSTD_FLOOR_H
#define NOSTD_FLOOR_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat floor(const puq::MagnitudeFloat& m);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType floor(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude floor(const puq::Magnitude& m);
#endif

} // namespace snt::nostd

#endif // NOSTD_FLOOR_H
