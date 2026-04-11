#ifndef NOSTD_MAX_H
#define NOSTD_MAX_H

#include <snt/puq/math/math.h>

namespace snt::math {

  extern double max(const double m1, const double m2);
  extern val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2);
  extern puq::Result max(const puq::Result& m1, const puq::Result& m2);
  extern puq::Measurement max(const puq::Measurement& msr1, const puq::Measurement& msr2);

} // namespace snt::math

#endif // NOSTD_MAX_H
