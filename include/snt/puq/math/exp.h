#ifndef NOSTD_EXP_H
#define NOSTD_EXP_H

#include <snt/puq/math/math.h>

namespace snt::math {

  extern double exp(const double e);
  extern val::BaseValue::PointerType exp(val::BaseValue::PointerType e);
  extern puq::Result exp(const puq::Result& e);
  extern puq::Measurement exp(const puq::Measurement& uv);

} // namespace snt::math

#endif // NOSTD_EXP_H
