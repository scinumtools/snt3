#ifndef NOSTD_EXP_H
#define NOSTD_EXP_H

#include "nostd.h"

namespace snt::nostd {

  extern MagnitudeFloat exp(const MagnitudeFloat& e);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType exp(val::BaseValue::PointerType e);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude exp(const puq::Magnitude& e);
#endif
  extern puq::UnitValue exp(const puq::UnitValue& uv);

} // namespace snt::nostd

#endif // NOSTD_EXP_H
