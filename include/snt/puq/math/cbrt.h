#ifndef NOSTD_CBRT_H
#define NOSTD_CBRT_H

#include <snt/puq/math/math.h>

namespace snt::math {

  extern double cbrt(const double m);
  extern val::BaseValue::PointerType cbrt(val::BaseValue::PointerType a);
  extern puq::Result cbrt(const puq::Result& m);
  extern puq::Measurement cbrt(const puq::Measurement& msr);

} // namespace snt::math

#endif // NOSTD_CBRT_H
