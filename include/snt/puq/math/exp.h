#ifndef PUQ_MATH_EXP_H
#define PUQ_MATH_EXP_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the exp operation to this operand.
     * @param res Input operand.
     */
    extern puq::Result exp(const puq::Result& res);
    /** Apply the exp operation to this operand.
     * @param msr Input operand.
     */
    extern puq::Measurement exp(const puq::Measurement& msr);
    /** Apply the exp operation to this operand.
     * @param quant Input operand.
     */
    extern puq::Quantity exp(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_EXP_H
