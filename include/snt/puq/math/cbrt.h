#ifndef PUQ_MATH_CBRT_H
#define PUQ_MATH_CBRT_H

namespace snt::puq {
    class BaseUnits;
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the cbrt operation to this operand.
     * @param bu Input operand.
     */
    extern puq::BaseUnits cbrt(const puq::BaseUnits& bu);
    /** Apply the cbrt operation to this operand.
     * @param res Input operand.
     */
    extern puq::Result cbrt(const puq::Result& res);
    /** Apply the cbrt operation to this operand.
     * @param msr Input operand.
     */
    extern puq::Measurement cbrt(const puq::Measurement& msr);
    /** Apply the cbrt operation to this operand.
     * @param quant Input operand.
     */
    extern puq::Quantity cbrt(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_CBRT_H
