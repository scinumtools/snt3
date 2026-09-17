#ifndef PUQ_MATH_CEIL_H
#define PUQ_MATH_CEIL_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the ceil operation to this operand.
     * @param res Result whose estimate and uncertainty are transformed.
     */
    extern puq::Result ceil(const puq::Result& res);
    /** Apply the ceil operation to this operand.
     * @param msr Measurement whose value and uncertainty are transformed.
     */
    extern puq::Measurement ceil(const puq::Measurement& msr);
    /** Apply the ceil operation to this operand.
     * @param quant Quantity whose value and uncertainty are transformed.
     */
    extern puq::Quantity ceil(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_CEIL_H
