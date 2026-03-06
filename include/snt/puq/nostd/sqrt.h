#ifndef NOSTD_SQRT_H
#define NOSTD_SQRT_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat sqrt(const puq::MagnitudeFloat& m);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a);
#endif
  extern puq::Magnitude sqrt(const puq::Magnitude& m);

} // namespace snt::nostd

#endif // NOSTD_SQRT_H
