#ifndef PUQ_MATH_CEIL_H
#define PUQ_MATH_CEIL_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the ceil operation to this operand.
     * @param res Input operand.
     */
    extern puq::Result ceil(const puq::Result& res);
    /** Apply the ceil operation to this operand.
     * @param msr Input operand.
     */
    extern puq::Measurement ceil(const puq::Measurement& msr);
    /** Apply the ceil operation to this operand.
     * @param quant Input operand.
     */
    extern puq::Quantity ceil(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_CEIL_H
