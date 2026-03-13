#ifndef NOSTD_ABS_H
#define NOSTD_ABS_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern double abs(const double m);
  extern val::BaseValue::PointerType abs(val::BaseValue::PointerType a);
  extern puq::Result abs(const puq::Result& m);
  extern puq::Measurement abs(const puq::Measurement& msr);

} // namespace snt::nostd

#endif // NOSTD_ABS_H
