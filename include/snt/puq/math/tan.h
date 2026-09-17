#ifndef PUQ_MATH_TAN_H
#define PUQ_MATH_TAN_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the tan operation to this operand.
     * @param res Input operand.
     */
    extern puq::Result tan(const puq::Result& res);
    /** Apply the tan operation to this operand.
     * @param msr Input operand.
     */
    extern puq::Measurement tan(const puq::Measurement& msr);
    /** Apply the tan operation to this operand.
     * @param quant Input operand.
     */
    extern puq::Quantity tan(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_TAN_H
