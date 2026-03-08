#ifndef NOSTD_ABS_H
#define NOSTD_ABS_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat abs(const puq::MagnitudeFloat& m);
  extern val::BaseValue::PointerType abs(val::BaseValue::PointerType a);
  extern puq::Magnitude abs(const puq::Magnitude& m);

} // namespace snt::nostd

#endif // NOSTD_ABS_H
