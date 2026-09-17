#ifndef PUQ_MATH_MAX_H
#define PUQ_MATH_MAX_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the max operation to this operand.
     * @param res1 First result operand.
     * @param res2 Second result operand.
     */
    extern puq::Result max(const puq::Result& res1, const puq::Result& res2);
    /** Apply the max operation to this operand.
     * @param msr1 First measurement operand.
     * @param msr2 Second measurement operand.
     */
    extern puq::Measurement max(const puq::Measurement& msr1, const puq::Measurement& msr2);
    /** Apply the max operation to this operand.
     * @param quant1 First quantity operand.
     * @param quant2 Second quantity operand.
     */
    extern puq::Quantity max(const puq::Quantity& quant1, const puq::Quantity& quant2);

} // namespace snt::puq::math

#endif // PUQ_MATH_MAX_H
