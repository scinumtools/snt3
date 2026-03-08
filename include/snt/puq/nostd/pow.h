#ifndef NOSTD_POW_H
#define NOSTD_POW_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern puq::MagnitudeFloat pow(const puq::MagnitudeFloat& m, const puq::ExponentFloat& e);
  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, const puq::ExponentFloat& e);
  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e);
  extern puq::Magnitude pow(const puq::Magnitude& m, const puq::ExponentFloat& e);
  extern puq::Magnitude pow(const puq::Magnitude& m, const puq::Magnitude& e);

} // namespace snt::nostd

#endif // NOSTD_POW_H
