#ifndef NOSTD_LOG_H
#define NOSTD_LOG_H

#include <snt/puq/math/math.h>

namespace snt::math {

  extern double log(const double m);
  extern val::BaseValue::PointerType log(val::BaseValue::PointerType a);
  extern puq::Result log(const puq::Result& m);
  extern puq::Measurement log(const puq::Measurement& msr);

} // namespace snt::math

#endif // NOSTD_LOG_H
