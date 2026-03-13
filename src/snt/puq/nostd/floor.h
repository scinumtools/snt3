#ifndef NOSTD_FLOOR_H
#define NOSTD_FLOOR_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern double floor(const double m);
  extern val::BaseValue::PointerType floor(val::BaseValue::PointerType a);
  extern puq::Magnitude floor(const puq::Magnitude& m);

} // namespace snt::nostd

#endif // NOSTD_FLOOR_H
