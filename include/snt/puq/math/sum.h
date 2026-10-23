/** @file Unit-aware reduction operations for PUQ values. */
#ifndef PUQ_MATH_SUM_H
#define PUQ_MATH_SUM_H

namespace snt::puq {
    class Result;
    class Measurement;
    class Quantity;
} // namespace snt::puq

namespace snt::puq::math {

    /** Sum every estimate in a result and linearly combine its absolute uncertainties.
     * @param result Result whose elements are reduced to one scalar.
     */
    extern puq::Result sum(const puq::Result& result);
    /** Sum every estimate in a measurement while preserving its base units.
     * @param measurement Measurement whose elements are reduced to one scalar.
     */
    extern puq::Measurement sum(const puq::Measurement& measurement);
    /** Sum every estimate in a quantity while preserving its units and unit system.
     * @param quantity Quantity whose elements are reduced to one scalar.
     */
    extern puq::Quantity sum(const puq::Quantity& quantity);

} // namespace snt::puq::math

#endif // PUQ_MATH_SUM_H
