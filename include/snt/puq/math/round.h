#ifndef PUQ_MATH_ROUND_H
#define PUQ_MATH_ROUND_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

  extern puq::Result round(const puq::Result& res);
  extern puq::Measurement round(const puq::Measurement& msr);
  extern puq::Quantity round(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_ROUND_H
