#ifndef NOSTD_LOG_H
#define NOSTD_LOG_H

#include "nostd.h"

namespace snt::nostd {

  extern puq::MagnitudeFloat log(const puq::MagnitudeFloat& m);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType log(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude log(const puq::Magnitude& m);
#endif

} // namespace snt::nostd

#endif // NOSTD_LOG_H
