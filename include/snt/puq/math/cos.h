#ifndef PUQ_MATH_COS_H
#define PUQ_MATH_COS_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the cos operation to this operand.
     * @param res Input operand.
     */
    extern puq::Result cos(const puq::Result& res);
    /** Apply the cos operation to this operand.
     * @param msr Input operand.
     */
    extern puq::Measurement cos(const puq::Measurement& msr);
    /** Apply the cos operation to this operand.
     * @param quant Input operand.
     */
    extern puq::Quantity cos(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_COS_H
