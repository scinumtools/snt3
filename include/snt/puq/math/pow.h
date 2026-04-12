#ifndef PUQ_MATH_POW_H
#define PUQ_MATH_POW_H

#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>

namespace snt::puq::math {

  extern puq::Result pow(const puq::Result& m, const double e);
  extern puq::Result pow(const puq::Result& m, const puq::Result& e);
  extern puq::Measurement pow(const puq::Measurement& msr, const double e);
  extern puq::Measurement pow(const puq::Measurement& msr1, const puq::Measurement& msr2);

} // namespace snt::puq::math

#endif // PUQ_MATH_POW_H
