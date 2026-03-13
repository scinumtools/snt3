#ifndef NOSTD_CBRT_H
#define NOSTD_CBRT_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::double cbrt(const puq::double& m);
  extern val::BaseValue::PointerType cbrt(val::BaseValue::PointerType a);
  extern puq::Magnitude cbrt(const puq::Magnitude& m);
  extern puq::Measurement cbrt(const puq::Measurement& msr);

} // namespace snt::nostd

#endif // NOSTD_CBRT_H
