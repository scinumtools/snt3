#ifndef PUQ_MATH_ROUND_H
#define PUQ_MATH_ROUND_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the round operation to this operand.
     * @param res Result whose estimate and uncertainty are transformed.
     */
    extern puq::Result round(const puq::Result& res);
    /** Apply the round operation to this operand.
     * @param msr Measurement whose value and uncertainty are transformed.
     */
    extern puq::Measurement round(const puq::Measurement& msr);
    /** Apply the round operation to this operand.
     * @param quant Quantity whose value and uncertainty are transformed.
     */
    extern puq::Quantity round(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_ROUND_H
