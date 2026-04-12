#ifndef PUQ_MATH_MAX_H
#define PUQ_MATH_MAX_H

#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>

namespace snt::puq::math {

  extern puq::Result max(const puq::Result& m1, const puq::Result& m2);
  extern puq::Measurement max(const puq::Measurement& msr1, const puq::Measurement& msr2);

} // namespace snt::puq::math

#endif // PUQ_MATH_MAX_H
