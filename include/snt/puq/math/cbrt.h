#ifndef PUQ_MATH_CBRT_H
#define PUQ_MATH_CBRT_H

#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>

namespace snt::puq::math {

  extern puq::Result cbrt(const puq::Result& m);
  extern puq::Measurement cbrt(const puq::Measurement& msr);

} // namespace snt::puq::math

#endif // PUQ_MATH_CBRT_H
