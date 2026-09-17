#ifndef PUQ_MATH_POW_H
#define PUQ_MATH_POW_H

#include <snt/puq/exponent.h>

namespace snt::puq {
    class BaseUnits;
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Apply the pow operation to this operand.
     * @param bu Input operand.
     * @param exp Input operand.
     */
    extern puq::BaseUnits pow(puq::BaseUnits bu, const ExponentVariant& exp);
    /** Apply the pow operation to this operand.
     * @param res Input operand.
     * @param exp Input operand.
     */
    extern puq::Result pow(const puq::Result& res, const double exp);
    /** Apply the pow operation to this operand.
     * @param res1 Input operand.
     * @param res2 Input operand.
     */
    extern puq::Result pow(const puq::Result& res1, const puq::Result& res2);
    /** Apply the pow operation to this operand.
     * @param msr Input operand.
     * @param exp Input operand.
     */
    extern puq::Measurement pow(const puq::Measurement& msr, const double exp);
    /** Apply the pow operation to this operand.
     * @param msr Input operand.
     * @param exp Input operand.
     */
    extern puq::Measurement pow(const puq::Measurement& msr, const ExponentVariant& exp);
    /** Apply the pow operation to this operand.
     * @param msr1 Input operand.
     * @param msr2 Input operand.
     */
    extern puq::Measurement pow(const puq::Measurement& msr1, const puq::Measurement& msr2);
    /** Apply the pow operation to this operand.
     * @param quant1 Input operand.
     * @param exp Input operand.
     */
    extern puq::Quantity pow(const puq::Quantity& quant1, const double exp);
    /** Apply the pow operation to this operand.
     * @param quant1 Input operand.
     * @param quant2 Input operand.
     */
    extern puq::Quantity pow(const puq::Quantity& quant1, const puq::Quantity& quant2);

} // namespace snt::puq::math

#endif // PUQ_MATH_POW_H
