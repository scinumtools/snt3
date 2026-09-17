#ifndef PUQ_MATH_LOG_H
#define PUQ_MATH_LOG_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the log operation to this operand.
     * @param res Input operand.
     */
    extern puq::Result log(const puq::Result& res);
    /** Apply the log operation to this operand.
     * @param msr Input operand.
     */
    extern puq::Measurement log(const puq::Measurement& msr);
    /** Apply the log operation to this operand.
     * @param quant Input operand.
     */
    extern puq::Quantity log(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_LOG_H
