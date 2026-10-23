#include <numeric>
#include <snt/puq/math/sum.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

namespace snt::puq::math {

    namespace {

        double sum_values(const val::BaseValue* value) {
            // Most PUQ quantities already hold Float64 arrays. Read those in place;
            // retain conversion support for other numeric VAL storage types.
            std::unique_ptr<val::ArrayValueFloat64> converted;
            const auto* values = dynamic_cast<const val::ArrayValueFloat64*>(value);
            if (!values) {
                converted = std::make_unique<val::ArrayValueFloat64>(value);
                values = converted.get();
            }
            if (values->get_size() == 0)
                return 0.0;
            const double* data = values->get_data();
            return std::accumulate(data, data + values->get_size(), 0.0);
        }

    } // namespace

    puq::Result sum(const puq::Result& result) {
        const double estimate = sum_values(result.estimate.get());
        if (!result.uncertainty)
            return puq::Result(estimate);

        return puq::Result(estimate, sum_values(result.uncertainty.get()));
    }

    puq::Measurement sum(const puq::Measurement& measurement) {
        return puq::Measurement(sum(measurement.result), measurement.baseunits);
    }

    puq::Quantity sum(const puq::Quantity& quantity) {
        return puq::Quantity(sum(quantity.measurement), quantity.stype);
    }

} // namespace snt::puq::math
