#ifndef NOSTD_LOG_H
#define NOSTD_LOG_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern double log(const double m);
  extern val::BaseValue::PointerType log(val::BaseValue::PointerType a);
  extern puq::Result log(const puq::Result& m);
  extern puq::Measurement log(const puq::Measurement& msr);

} // namespace snt::nostd

#endif // NOSTD_LOG_H
