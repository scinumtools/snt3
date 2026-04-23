#ifndef PUQ_MATH_CEIL_H
#define PUQ_MATH_CEIL_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

  extern puq::Result ceil(const puq::Result& res);
  extern puq::Measurement ceil(const puq::Measurement& msr);
  extern puq::Quantity ceil(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_CEIL_H
