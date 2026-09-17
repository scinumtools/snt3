#ifndef PUQ_MATH_ABS_H
#define PUQ_MATH_ABS_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Return the absolute value of a PUQ result. */
    extern puq::Result abs(const puq::Result& res);
    /** Return the absolute value of a PUQ measurement. */
    extern puq::Measurement abs(const puq::Measurement& msr);
    /** Return the absolute value of a PUQ quantity. */
    extern puq::Quantity abs(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_ABS_H
