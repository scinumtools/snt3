#ifndef PUQ_MATH_LOG10_H
#define PUQ_MATH_LOG10_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

  extern puq::Result log10(const puq::Result& m);
  extern puq::Measurement log10(const puq::Measurement& msr);
  extern puq::Quantity log10(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_LOG10_H
