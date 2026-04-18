#ifndef PUQ_MATH_SQRT_H
#define PUQ_MATH_SQRT_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
}

namespace snt::puq::math {

  extern puq::Result sqrt(const puq::Result& m);
  extern puq::Measurement sqrt(const puq::Measurement& msr);
  extern puq::Quantity sqrt(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_SQRT_H
