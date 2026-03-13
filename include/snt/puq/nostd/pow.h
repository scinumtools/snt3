#ifndef NOSTD_POW_H
#define NOSTD_POW_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern double pow(const double& m, const double& e);
  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, const double& e);
  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e);
  extern puq::Magnitude pow(const puq::Magnitude& m, const double& e);
  extern puq::Magnitude pow(const puq::Magnitude& m, const puq::Magnitude& e);
  extern puq::Measurement pow(const puq::Measurement& msr, const double& e);
  extern puq::Measurement pow(const puq::Measurement& msr1, const puq::Measurement& msr2);

} // namespace snt::nostd

#endif // NOSTD_POW_H
