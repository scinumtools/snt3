#ifndef PUQ_MATH_POW_H
#define PUQ_MATH_POW_H

namespace snt::puq {
  class Result;
  class Measurement;
  class Quantity;
}

namespace snt::puq::math {

  extern puq::Result pow(const puq::Result& res, const double exp);
  extern puq::Result pow(const puq::Result& res1, const puq::Result& res2);
  extern puq::Measurement pow(const puq::Measurement& msr, const double exp);
  extern puq::Measurement pow(const puq::Measurement& msr1, const puq::Measurement& msr2);
  extern puq::Quantity pow(const puq::Quantity& quant1, const double exp);
  extern puq::Quantity pow(const puq::Quantity& quant1, const puq::Quantity& quant2);

} // namespace snt::puq::math

#endif // PUQ_MATH_POW_H
