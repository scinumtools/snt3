#ifndef PUQ_MATH_SQRT_H
#define PUQ_MATH_SQRT_H

namespace snt::puq {
    class BaseUnits;
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the sqrt operation to this operand.
     * @param bu Input operand.
     */
    extern puq::BaseUnits sqrt(const puq::BaseUnits& bu);
    /** Apply the sqrt operation to this operand.
     * @param res Input operand.
     */
    extern puq::Result sqrt(const puq::Result& res);
    /** Apply the sqrt operation to this operand.
     * @param msr Input operand.
     */
    extern puq::Measurement sqrt(const puq::Measurement& msr);
    /** Apply the sqrt operation to this operand.
     * @param quant Input operand.
     */
    extern puq::Quantity sqrt(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_SQRT_H
