#ifndef NOSTD_EXP_H
#define NOSTD_EXP_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat exp(const puq::MagnitudeFloat& e);
#if defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType exp(val::BaseValue::PointerType e);
#endif
#ifdef MAGNITUDE_UNCERTAINTIES
  extern puq::Magnitude exp(const puq::Magnitude& e);
#endif
  extern puq::Measurement exp(const puq::Measurement& uv);

} // namespace snt::nostd

#endif // NOSTD_EXP_H
