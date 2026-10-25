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
            const std::size_t size = values->get_size();

            // Independent partial sums break the dependency chain in a serial
            // accumulation. This lets optimizing compilers process several
            // values at once while keeping a fixed, reproducible reduction
            // order. The final combination follows the original element order
            // within each lane; only the grouping changes.
            double sum0 = 0.0;
            double sum1 = 0.0;
            double sum2 = 0.0;
            double sum3 = 0.0;
            double sum4 = 0.0;
            double sum5 = 0.0;
            double sum6 = 0.0;
            double sum7 = 0.0;
            std::size_t i = 0;
            for (; i + 7 < size; i += 8) {
                sum0 += data[i];
                sum1 += data[i + 1];
                sum2 += data[i + 2];
                sum3 += data[i + 3];
                sum4 += data[i + 4];
                sum5 += data[i + 5];
                sum6 += data[i + 6];
                sum7 += data[i + 7];
            }

            double sum = ((sum0 + sum1) + (sum2 + sum3)) + ((sum4 + sum5) + (sum6 + sum7));
            for (; i < size; ++i)
                sum += data[i];
            return sum;
        }

    } // namespace

    puq::Result sum(const puq::Result& result) {
        puq::Result output(sum_values(result.estimate.get()));
        for (const auto& [id, contribution] : result.sources)
            output.sources.emplace(id, std::make_unique<val::ArrayValueFloat64>(sum_values(contribution.get())));
        output.refresh_uncertainty();
        return output;
    }

    puq::Measurement sum(const puq::Measurement& measurement) {
        return puq::Measurement(sum(measurement.result), measurement.baseunits);
    }

    puq::Quantity sum(const puq::Quantity& quantity) {
        return puq::Quantity(sum(quantity.measurement), quantity.stype);
    }

} // namespace snt::puq::math
