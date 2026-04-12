#ifndef PUQ_MATH_LOG10_H
#define PUQ_MATH_LOG10_H

#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>

namespace snt::puq::math {

  extern puq::Result log10(const puq::Result& m);
  extern puq::Measurement log10(const puq::Measurement& msr);

} // namespace snt::puq::math

#endif // PUQ_MATH_LOG10_H
