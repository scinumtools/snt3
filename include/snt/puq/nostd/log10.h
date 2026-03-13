#ifndef NOSTD_LOG10_H
#define NOSTD_LOG10_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern double log10(const double m);
  extern val::BaseValue::PointerType log10(val::BaseValue::PointerType a);
  extern puq::Result log10(const puq::Result& m);
  extern puq::Measurement log10(const puq::Measurement& msr);

} // namespace snt::nostd

#endif // NOSTD_LOG10_H
