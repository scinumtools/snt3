#ifndef NOSTD_FLOOR_H
#define NOSTD_FLOOR_H

namespace snt::nostd {

  extern MAGNITUDE_PRECISION floor(const MAGNITUDE_PRECISION& m);
#if defined(MAGNITUDE_ARRAYS)
  extern Array floor(const Array& a);
#elif defined(MAGNITUDE_VALUES)
  extern val::BaseValue::PointerType floor(val::BaseValue::PointerType a);
#endif
#ifdef MAGNITUDE_ERRORS
  extern puq::Magnitude floor(const puq::Magnitude& m);
#endif

} // namespace snt::nostd

#endif // NOSTD_FLOOR_H
