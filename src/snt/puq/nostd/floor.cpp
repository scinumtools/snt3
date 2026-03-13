#include <snt/puq/nostd/floor.h>

namespace snt::nostd {

  double floor(const double m) {
    return std::floor(m);
  }

  val::BaseValue::PointerType floor(val::BaseValue::PointerType a) {
    return a->math_floor();
  }

  puq::Result floor(const puq::Result& m) {
    return puq::Result(m.estimate->math_floor());
    //return puq::Result(floor(m.estimate));
  }

} // namespace snt::nostd
