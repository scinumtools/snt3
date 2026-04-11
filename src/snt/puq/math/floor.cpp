#include <snt/puq/math/floor.h>

namespace snt::math {

  double floor(const double m) {
    return std::floor(m);
  }

  val::BaseValue::PointerType floor(val::BaseValue::PointerType a) {
    return a->math_floor();
  }

  puq::Result floor(const puq::Result& m) {
    return puq::Result(m.estimate->math_floor());
    // return puq::Result(floor(m.estimate));
  }

} // namespace snt::math
