#ifndef PUQ_MATH_SQRT_H
#define PUQ_MATH_SQRT_H

#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>

namespace snt::puq::math {

  extern puq::Result sqrt(const puq::Result& m);
  extern puq::Measurement sqrt(const puq::Measurement& msr);

} // namespace snt::puq::math

#endif // PUQ_MATH_SQRT_H
