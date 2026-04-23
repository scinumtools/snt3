#ifndef PUQ_MATH_SQRT_H
#define PUQ_MATH_SQRT_H

namespace snt::puq {
  class BaseUnits;
  class Result;
  class Measurement;
  class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

  extern puq::BaseUnits sqrt(const puq::BaseUnits& bu);
  extern puq::Result sqrt(const puq::Result& res);
  extern puq::Measurement sqrt(const puq::Measurement& msr);
  extern puq::Quantity sqrt(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_SQRT_H
