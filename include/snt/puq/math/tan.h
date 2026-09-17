#ifndef PUQ_MATH_TAN_H
#define PUQ_MATH_TAN_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the tan operation to this operand.
     * @param res Result whose estimate and uncertainty are transformed.
     */
    extern puq::Result tan(const puq::Result& res);
    /** Apply the tan operation to this operand.
     * @param msr Measurement whose value and uncertainty are transformed.
     */
    extern puq::Measurement tan(const puq::Measurement& msr);
    /** Apply the tan operation to this operand.
     * @param quant Quantity whose value and uncertainty are transformed.
     */
    extern puq::Quantity tan(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_TAN_H
