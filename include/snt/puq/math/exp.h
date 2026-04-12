#ifndef PUQ_MATH_EXP_H
#define PUQ_MATH_EXP_H

#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>

namespace snt::puq::math {

  extern puq::Result exp(const puq::Result& e);
  extern puq::Measurement exp(const puq::Measurement& uv);

} // namespace snt::puq::math

#endif // PUQ_MATH_EXP_H
