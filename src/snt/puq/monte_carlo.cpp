#include <snt/puq/monte_carlo.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <stdexcept>
#include <string>

namespace snt::puq {

    namespace {
        double scalar(const val::BaseValue* value) {
            if (!value || value->get_size() != 1)
                throw std::invalid_argument("Monte Carlo inputs must have scalar values and uncertainties.");
            const val::ArrayValueFloat64 converted(value);
            return converted.get_value(0);
        }

        double quantile(const std::vector<double>& sorted, double probability) {
            const double index = probability * static_cast<double>(sorted.size() - 1);
            const auto lower = static_cast<std::size_t>(index);
            const auto upper = std::min(lower + 1, sorted.size() - 1);
            return sorted[lower] + (sorted[upper] - sorted[lower]) * (index - static_cast<double>(lower));
        }
    } // namespace

    MonteCarloResult monte_carlo(
        const std::vector<Quantity>& inputs,
        const std::vector<InputDistribution>& distributions,
        const std::function<Quantity(const std::vector<Quantity>&)>& model,
        std::size_t samples,
        std::uint64_t seed,
        double coverage
    ) {
        if (!model || inputs.empty() || inputs.size() != distributions.size())
            throw std::invalid_argument("Provide a model and one distribution for each input.");
        if (samples < 2)
            throw std::invalid_argument("Monte Carlo propagation requires at least two samples.");
        if (!std::isfinite(coverage) || coverage <= 0.0 || coverage >= 1.0)
            throw std::invalid_argument("Coverage must be strictly between zero and one.");

        // A source ID is shared by copies and by explicitly correlated inputs.
        // Draw it only once to preserve that relationship in every model evaluation.
        std::map<std::size_t, InputDistribution> source_distributions;
        for (std::size_t i = 0; i < inputs.size(); ++i) {
            const auto& result = inputs[i].measurement.result;
            scalar(result.estimate.get());
            result.ensure_source();
            if (distributions[i] == InputDistribution::Uniform && result.sources.size() > 1)
                throw std::invalid_argument("A uniform input must have one primitive uncertainty source.");
            for (const auto& [id, contribution] : result.sources) {
                scalar(contribution.get());
                const auto [it, inserted] = source_distributions.emplace(id, distributions[i]);
                if (!inserted && it->second != distributions[i])
                    throw std::invalid_argument("A shared uncertainty source cannot have different distributions.");
            }
        }

        std::mt19937_64 engine(seed);
        std::normal_distribution<double> normal(0.0, 1.0);
        std::uniform_real_distribution<double> uniform(-std::sqrt(3.0), std::sqrt(3.0));
        std::map<std::size_t, std::vector<double>> draws;
        for (const auto& [id, distribution] : source_distributions) {
            auto& values = draws[id];
            values.reserve(samples);
            for (std::size_t k = 0; k < samples; ++k)
                values.push_back(distribution == InputDistribution::Normal ? normal(engine) : uniform(engine));
        }

        std::vector<Quantity> sampled_inputs;
        sampled_inputs.reserve(inputs.size());
        for (const auto& input : inputs) {
            const auto& result = input.measurement.result;
            std::vector<double> values(samples, scalar(result.estimate.get()));
            for (const auto& [id, contribution] : result.sources) {
                const double scale = scalar(contribution.get());
                const auto& source_draws = draws.at(id);
                for (std::size_t k = 0; k < samples; ++k)
                    values[k] += scale * source_draws[k];
            }
            auto array = std::make_unique<val::ArrayValueFloat64>(std::move(values), val::Array::ShapeType{samples});
            sampled_inputs.emplace_back(Measurement(std::move(array), input.measurement.baseunits), input.stype);
        }

        const Quantity output = model(sampled_inputs);
        const val::ArrayValueFloat64 output_values(output.measurement.result.estimate.get());
        if (output_values.get_size() != samples)
            throw std::invalid_argument("Monte Carlo model must return one value per sample.");
        std::vector<double> values = output_values.get_values();
        double mean = 0.0;
        double m2 = 0.0;
        for (std::size_t k = 0; k < samples; ++k) {
            if (!std::isfinite(values[k]))
                throw std::domain_error("Monte Carlo model produced a non-finite value at sample " + std::to_string(k) + ".");
            const double delta = values[k] - mean;
            mean += delta / static_cast<double>(k + 1);
            m2 += delta * (values[k] - mean);
        }
        const double deviation = std::sqrt(m2 / static_cast<double>(samples - 1));
        std::sort(values.begin(), values.end());
        const double tail = (1.0 - coverage) / 2.0;
        return {
            Quantity(Measurement(mean, deviation, output.measurement.baseunits), output.stype),
            quantile(values, tail),
            quantile(values, 1.0 - tail),
            coverage,
            samples
        };
    }

} // namespace snt::puq
