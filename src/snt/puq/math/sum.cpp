#include <numeric>
#include <snt/puq/math/sum.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

    puq::Result sum(const puq::Result& result) {
        const val::ArrayValueFloat64 estimates(result.estimate.get());
        const auto values = estimates.get_values();
        const double estimate = std::accumulate(values.begin(), values.end(), 0.0);
        if (!result.uncertainty)
            return puq::Result(estimate);

        const val::ArrayValueFloat64 uncertainties(result.uncertainty.get());
        const auto errors = uncertainties.get_values();
        return puq::Result(estimate, std::accumulate(errors.begin(), errors.end(), 0.0));
    }

    puq::Measurement sum(const puq::Measurement& measurement) {
        return puq::Measurement(sum(measurement.result), measurement.baseunits);
    }

    puq::Quantity sum(const puq::Quantity& quantity) {
        return puq::Quantity(sum(quantity.measurement), quantity.stype);
    }

} // namespace snt::puq::math
