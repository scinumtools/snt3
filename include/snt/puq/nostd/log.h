#ifndef NOSTD_LOG_H
#define NOSTD_LOG_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat log(const puq::MagnitudeFloat& m);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType log(val::BaseValue::PointerType a);
#endif
  extern puq::Magnitude log(const puq::Magnitude& m);

} // namespace snt::nostd

#endif // NOSTD_LOG_H
