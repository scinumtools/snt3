#include <snt/puq/math/floor.h>

namespace snt::puq::math {

  puq::Result floor(const puq::Result& m) {
    return puq::Result(m.estimate->math_floor());
    // return puq::Result(floor(m.estimate));
  }

} // namespace snt::puq::math
