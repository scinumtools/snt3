#ifndef PUQ_MATH_TAN_H
#define PUQ_MATH_TAN_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

  extern puq::Result tan(const puq::Result& res);
  extern puq::Measurement tan(const puq::Measurement& msr);
  extern puq::Quantity tan(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_TAN_H
