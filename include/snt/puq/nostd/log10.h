#ifndef NOSTD_LOG10_H
#define NOSTD_LOG10_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat log10(const puq::MagnitudeFloat& m);
  extern val::BaseValue::PointerType log10(val::BaseValue::PointerType a);
  extern puq::Magnitude log10(const puq::Magnitude& m);

} // namespace snt::nostd

#endif // NOSTD_LOG10_H
