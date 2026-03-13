#ifndef NOSTD_SQRT_H
#define NOSTD_SQRT_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern double sqrt(const double m);
  extern val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a);
  extern puq::Magnitude sqrt(const puq::Magnitude& m);
  extern puq::Measurement sqrt(const puq::Measurement& msr);

} // namespace snt::nostd

#endif // NOSTD_SQRT_H
