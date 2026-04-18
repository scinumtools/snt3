#ifndef PUQ_MATH_FLOOR_H
#define PUQ_MATH_FLOOR_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
}

namespace snt::puq::math {

  extern puq::Result floor(const puq::Result& res);
  extern puq::Measurement floor(const puq::Measurement& msr);
  extern puq::Quantity floor(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_FLOOR_H
