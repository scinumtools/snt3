#ifndef NOSTD_CBRT_H
#define NOSTD_CBRT_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat cbrt(const puq::MagnitudeFloat& m);
  extern val::BaseValue::PointerType cbrt(val::BaseValue::PointerType a);
  extern puq::Magnitude cbrt(const puq::Magnitude& m);

} // namespace snt::nostd

#endif // NOSTD_CBRT_H
