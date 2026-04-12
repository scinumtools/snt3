#ifndef PUQ_MATH_ABS_H
#define PUQ_MATH_ABS_H

#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>

namespace snt::puq::math {

  extern puq::Result abs(const puq::Result& m);
  extern puq::Measurement abs(const puq::Measurement& msr);

} // namespace snt::puq::math

#endif // PUQ_MATH_ABS_H
