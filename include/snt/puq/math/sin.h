#ifndef PUQ_MATH_SIN_H
#define PUQ_MATH_SIN_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the sin operation to this operand.
     * @param res Result whose estimate and uncertainty are transformed.
     */
    extern puq::Result sin(const puq::Result& res);
    /** Apply sine to a dimensionless value or angle, converting angles to radians.
     * Input uncertainty is converted to radians before propagation; the result is dimensionless.
     * @param msr Measurement whose value and uncertainty are transformed.
     */
    extern puq::Measurement sin(const puq::Measurement& msr);
    /** Apply sine to a dimensionless value or angle, converting angles to radians.
     * Input uncertainty is converted to radians before propagation; the result is dimensionless.
     * @param quant Quantity whose value and uncertainty are transformed.
     */
    extern puq::Quantity sin(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_SIN_H
