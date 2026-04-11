#ifndef NOSTD_LOG10_H
#define NOSTD_LOG10_H

#include <snt/puq/math/math.h>

namespace snt::math {

  extern double log10(const double m);
  extern val::BaseValue::PointerType log10(val::BaseValue::PointerType a);
  extern puq::Result log10(const puq::Result& m);
  extern puq::Measurement log10(const puq::Measurement& msr);

} // namespace snt::math

#endif // NOSTD_LOG10_H
