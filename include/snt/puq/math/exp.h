#ifndef PUQ_MATH_EXP_H
#define PUQ_MATH_EXP_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
}

namespace snt::puq::math {

  extern puq::Result exp(const puq::Result& res);
  extern puq::Measurement exp(const puq::Measurement& msr);
  extern puq::Quantity exp(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_EXP_H
