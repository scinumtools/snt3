#ifndef PUQ_MATH_LOG_H
#define PUQ_MATH_LOG_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

  extern puq::Result log(const puq::Result& res);
  extern puq::Measurement log(const puq::Measurement& msr);
  extern puq::Quantity log(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_LOG_H
