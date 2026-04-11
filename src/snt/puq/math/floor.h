#ifndef NOSTD_FLOOR_H
#define NOSTD_FLOOR_H

#include <snt/puq/math/math.h>

namespace snt::math {

  extern double floor(const double m);
  extern val::BaseValue::PointerType floor(val::BaseValue::PointerType a);
  extern puq::Result floor(const puq::Result& m);

} // namespace snt::math

#endif // NOSTD_FLOOR_H
