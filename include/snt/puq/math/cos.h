#ifndef PUQ_MATH_COS_H
#define PUQ_MATH_COS_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

  extern puq::Result cos(const puq::Result& res);
  extern puq::Measurement cos(const puq::Measurement& msr);
  extern puq::Quantity cos(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_COS_H
