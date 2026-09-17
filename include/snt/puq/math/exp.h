#ifndef PUQ_MATH_EXP_H
#define PUQ_MATH_EXP_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the exp operation to this operand.
     * @param res Result whose estimate and uncertainty are transformed.
     */
    extern puq::Result exp(const puq::Result& res);
    /** Apply the exp operation to this operand.
     * @param msr Measurement whose value and uncertainty are transformed.
     */
    extern puq::Measurement exp(const puq::Measurement& msr);
    /** Apply the exp operation to this operand.
     * @param quant Quantity whose value and uncertainty are transformed.
     */
    extern puq::Quantity exp(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_EXP_H
