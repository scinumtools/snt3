#ifndef PUQ_MATH_MAX_H
#define PUQ_MATH_MAX_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
}

namespace snt::puq::math {

  extern puq::Result max(const puq::Result& res1, const puq::Result& res2);
  extern puq::Measurement max(const puq::Measurement& msr1, const puq::Measurement& msr2);
  extern puq::Quantity max(const puq::Quantity& quant1, const puq::Quantity& quant2);

} // namespace snt::puq::math

#endif // PUQ_MATH_MAX_H
