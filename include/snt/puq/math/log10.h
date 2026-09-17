#ifndef PUQ_MATH_LOG10_H
#define PUQ_MATH_LOG10_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the log10 operation to this operand.
     * @param m Input operand.
     */
    extern puq::Result log10(const puq::Result& m);
    /** Apply the log10 operation to this operand.
     * @param msr Input operand.
     */
    extern puq::Measurement log10(const puq::Measurement& msr);
    /** Apply the log10 operation to this operand.
     * @param quant Input operand.
     */
    extern puq::Quantity log10(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_LOG10_H
