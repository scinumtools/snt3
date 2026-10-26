#ifndef PUQ_MONTE_CARLO_H
#define PUQ_MONTE_CARLO_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <snt/puq/quantity.h>
#include <vector>

namespace snt::puq {

    enum class InputDistribution { Normal, Uniform };

    struct MonteCarloResult {
        Quantity quantity; ///< Sample mean and sample standard deviation in output units.
        double lower;      ///< Lower endpoint of the central coverage interval.
        double upper;      ///< Upper endpoint of the central coverage interval.
        double coverage;
        std::size_t samples;
    };

    /** Propagate scalar uncertain inputs through an ordinary Quantity calculation.
     *
     * Each primitive source is drawn once per sample and reused wherever it occurs.
     * Normal inputs may have multiple correlated source contributions. A uniform
     * input must have one primitive source. The model must return one scalar value
     * per sample; non-finite outputs are rejected rather than silently discarded.
     */
    MonteCarloResult monte_carlo(
        const std::vector<Quantity>& inputs,
        const std::vector<InputDistribution>& distributions,
        const std::function<Quantity(const std::vector<Quantity>&)>& model,
        std::size_t samples = 100000,
        std::uint64_t seed = 0,
        double coverage = 0.95
    );

} // namespace snt::puq

#endif
