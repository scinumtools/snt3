#ifndef PUQ_MATH_SIN_H
#define PUQ_MATH_SIN_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the sin operation to this operand.
     * @param res Input operand.
     */
    extern puq::Result sin(const puq::Result& res);
    /** Apply the sin operation to this operand.
     * @param msr Input operand.
     */
    extern puq::Measurement sin(const puq::Measurement& msr);
    /** Apply the sin operation to this operand.
     * @param quant Input operand.
     */
    extern puq::Quantity sin(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_SIN_H
