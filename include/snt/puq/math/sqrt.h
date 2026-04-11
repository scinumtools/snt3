#ifndef NOSTD_SQRT_H
#define NOSTD_SQRT_H

#include <snt/puq/math/math.h>

namespace snt::math {

  extern double sqrt(const double m);
  extern val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a);
  extern puq::Result sqrt(const puq::Result& m);
  extern puq::Measurement sqrt(const puq::Measurement& msr);

} // namespace snt::math

#endif // NOSTD_SQRT_H
