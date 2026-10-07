#ifndef PUQ_MATH_COS_H
#define PUQ_MATH_COS_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the cos operation to this operand.
     * @param res Result whose estimate and uncertainty are transformed.
     */
    extern puq::Result cos(const puq::Result& res);
    /** Apply cosine to a dimensionless value or angle, converting angles to radians.
     * Input uncertainty is converted to radians before propagation; the result is dimensionless.
     * @param msr Measurement whose value and uncertainty are transformed.
     */
    extern puq::Measurement cos(const puq::Measurement& msr);
    /** Apply cosine to a dimensionless value or angle, converting angles to radians.
     * Input uncertainty is converted to radians before propagation; the result is dimensionless.
     * @param quant Quantity whose value and uncertainty are transformed.
     */
    extern puq::Quantity cos(const puq::Quantity& quant);

} // namespace snt::puq::math

#endif // PUQ_MATH_COS_H
