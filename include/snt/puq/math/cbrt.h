#ifndef PUQ_MATH_CBRT_H
#define PUQ_MATH_CBRT_H

namespace snt::puq {
  class BaseUnits;
  class Result;
  class Measurement;
  class Quantity;
}

namespace snt::puq::math {

  extern puq::BaseUnits cbrt(const puq::BaseUnits& bu);
  extern puq::Result cbrt(const puq::Result& res);
  extern puq::Measurement cbrt(const puq::Measurement& msr);
  extern puq::Quantity cbrt(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_CBRT_H
