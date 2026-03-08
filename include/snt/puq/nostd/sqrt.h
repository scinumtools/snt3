#ifndef NOSTD_SQRT_H
#define NOSTD_SQRT_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat sqrt(const puq::MagnitudeFloat& m);
  extern val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a);
  extern puq::Magnitude sqrt(const puq::Magnitude& m);

} // namespace snt::nostd

#endif // NOSTD_SQRT_H
