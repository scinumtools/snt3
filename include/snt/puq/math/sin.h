#ifndef PUQ_MATH_SIN_H
#define PUQ_MATH_SIN_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
}

namespace snt::puq::math {

  extern puq::Result sin(const puq::Result& res);
  extern puq::Measurement sin(const puq::Measurement& msr);
  extern puq::Quantity sin(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_SIN_H
