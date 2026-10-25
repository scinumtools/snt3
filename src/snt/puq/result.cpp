#include <algorithm>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <snt/puq/exceptions.h>
#include <snt/puq/result.h>
#include <sstream>
#include <stdexcept>

namespace snt::puq {

    namespace {
        std::atomic_size_t next_source_id{1};
        std::atomic<Result::PropagationMode> active_mode{Result::PropagationMode::Standard};

        void add_scaled_sources(
            Result::SourceMap& target,
            const Result::SourceMap& source,
            const val::BaseValue* derivative
        ) {
            for (const auto& [id, contribution] : source) {
                auto scaled = derivative ? contribution->math_mul(derivative) : contribution->clone();
                if (auto it = target.find(id); it != target.end())
                    it->second->math_add_equal(scaled.get());
                else
                    target.emplace(id, std::move(scaled));
            }
        }
    } // namespace

    void Result::set_propagation_mode(PropagationMode mode) {
        active_mode.store(mode);
    }

    Result::PropagationMode Result::propagation_mode() {
        return active_mode.load();
    }

    Result::Result(val::BaseValue::PointerType m) : estimate(std::move(m)), uncertainty(nullptr) {
        if (!estimate)
            throw puq::ParserException(
                "Null result value",
                "The result value is a null pointer.",
                "Provide a valid result value.",
                __FILE__,
                __LINE__
            );
    }

    Result::Result(const double m, const double e)
        : Result(std::make_unique<val::ArrayValueFloat64>(m), std::make_unique<val::ArrayValueFloat64>(e)) {}

    Result::Result(val::BaseValue::PointerType m, val::BaseValue::PointerType e)
        : estimate(std::move(m)), uncertainty(std::move(e)) {
        if (!estimate)
            throw puq::ParserException(
                "Null result value",
                "The result value is a null pointer.",
                "Provide a valid result value.",
                __FILE__,
                __LINE__
            );
        if (uncertainty && estimate->get_size() != uncertainty->get_size())
            throw puq::ParserException(
                "Size mismatch",
                "The result value and its uncertainty have different sizes: `" + std::to_string(estimate->get_size()) +
                    "` != `" + std::to_string(uncertainty->get_size()) + "`.",
                "Provide an uncertainty value with the same number of elements as the result value.",
                __FILE__,
                __LINE__
            );
        if (uncertainty && uncertainty->any_of())
            sources.emplace(next_source_id++, uncertainty->clone());
    }

    void Result::refresh_uncertainty() {
        uncertainty.reset();
        for (const auto& [id, contribution] : sources) {
            auto square = contribution->math_pow(2);
            uncertainty = uncertainty ? uncertainty->math_add(square.get()) : std::move(square);
        }
        if (uncertainty)
            uncertainty = uncertainty->math_sqrt();
    }

    void Result::ensure_source() const {
        if (sources.empty() && uncertainty && uncertainty->any_of())
            sources.emplace(next_source_id++, uncertainty->clone());
    }

    Result Result::transformed(val::BaseValue::PointerType value, val::BaseValue::PointerType derivative) const {
        ensure_source();
        Result output(std::move(value));
        add_scaled_sources(output.sources, sources, derivative.get());
        output.refresh_uncertainty();
        return output;
    }

    void Result::correlate(Result& left, Result& right, double coefficient) {
        if (!std::isfinite(coefficient) || coefficient < -1.0 || coefficient > 1.0)
            throw std::invalid_argument("Correlation coefficient must be finite and in [-1, 1].");
        left.ensure_source();
        right.ensure_source();
        if (left.sources.size() != 1 || right.sources.size() != 1 || left.shape() != right.shape())
            throw std::invalid_argument("Correlation requires primitive uncertain results with matching shapes.");
        if (coefficient == 0.0)
            return;

        auto left_error = left.uncertainty->clone();
        auto right_error = right.uncertainty->clone();
        left.sources.clear();
        right.sources.clear();
        const double common_scale = std::sqrt(std::abs(coefficient));
        const double residual_scale = std::sqrt(1.0 - std::abs(coefficient));
        const auto common_id = next_source_id++;
        auto common_left = left_error->math_mul(std::make_unique<val::ArrayValueFloat64>(common_scale).get());
        auto common_right = right_error->math_mul(
            std::make_unique<val::ArrayValueFloat64>(coefficient < 0.0 ? -common_scale : common_scale).get()
        );
        left.sources.emplace(common_id, std::move(common_left));
        right.sources.emplace(common_id, std::move(common_right));
        if (residual_scale > 0.0) {
            auto scale = std::make_unique<val::ArrayValueFloat64>(residual_scale);
            left.sources.emplace(next_source_id++, left_error->math_mul(scale.get()));
            right.sources.emplace(next_source_id++, right_error->math_mul(scale.get()));
        }
        left.refresh_uncertainty();
        right.refresh_uncertainty();
    }

    void Result::correlate_many(const std::vector<Result*>& values, const std::vector<std::vector<double>>& covariance) {
        const std::size_t count = values.size();
        if (count == 0 || covariance.size() != count)
            throw std::invalid_argument("Covariance matrix dimensions must match the non-empty input list.");
        for (const auto& row : covariance)
            if (row.size() != count)
                throw std::invalid_argument("Covariance matrix must be square.");
        for (const auto* value : values)
            if (!value)
                throw std::invalid_argument("Covariance input list contains a null result.");

        const auto expected_shape = values.front()->shape();
        std::vector<double> stddev(count);
        for (std::size_t i = 0; i < count; ++i) {
            values[i]->ensure_source();
            if (!values[i] || values[i]->sources.size() != 1 || !values[i]->uncertainty ||
                values[i]->shape() != expected_shape || values[i]->size() != 1)
                throw std::invalid_argument("Covariance inputs must be scalar primitive uncertain results.");
            const auto* uncertainty = dynamic_cast<const val::ArrayValue<double>*>(values[i]->uncertainty.get());
            if (!uncertainty)
                throw std::invalid_argument("Covariance inputs must use floating-point uncertainties.");
            stddev[i] = uncertainty->get_value(0);
        }

        std::vector<std::vector<double>> factor(count, std::vector<double>(count, 0.0));
        for (std::size_t i = 0; i < count; ++i) {
            for (std::size_t j = 0; j < count; ++j) {
                if (!std::isfinite(covariance[i][j]) ||
                    std::abs(covariance[i][j] - covariance[j][i]) >
                        1e-12 * std::max({1e-300, std::abs(covariance[i][j]), std::abs(covariance[j][i])}))
                    throw std::invalid_argument("Covariance matrix must be finite and symmetric.");
            }
            if (covariance[i][i] < 0.0 ||
                std::abs(covariance[i][i] - stddev[i] * stddev[i]) >
                    1e-10 * std::max({1e-300, covariance[i][i], stddev[i] * stddev[i]}))
                throw std::invalid_argument("Covariance diagonal must match the input uncertainty variances.");
            for (std::size_t j = 0; j <= i; ++j) {
                double residual = covariance[i][j];
                for (std::size_t k = 0; k < j; ++k)
                    residual -= factor[i][k] * factor[j][k];
                if (i == j) {
                    if (residual < -1e-12 * std::max(1e-300, covariance[i][i]))
                        throw std::invalid_argument("Covariance matrix must be positive semidefinite.");
                    factor[i][j] = std::sqrt(std::max(0.0, residual));
                } else if (factor[j][j] > 0.0) {
                    factor[i][j] = residual / factor[j][j];
                } else if (std::abs(residual) > 1e-12) {
                    throw std::invalid_argument("Covariance matrix must be positive semidefinite.");
                }
            }
        }

        std::vector<std::size_t> source_ids(count);
        for (auto& id : source_ids)
            id = next_source_id++;
        for (std::size_t i = 0; i < count; ++i) {
            values[i]->sources.clear();
            if (stddev[i] > 0.0) {
                for (std::size_t k = 0; k <= i; ++k) {
                    if (factor[i][k] == 0.0)
                        continue;
                    auto scale = std::make_unique<val::ArrayValueFloat64>(factor[i][k] / stddev[i]);
                    values[i]->sources.emplace(source_ids[k], values[i]->uncertainty->math_mul(scale.get()));
                }
            }
            values[i]->refresh_uncertainty();
        }
    }

    /*
     * Convert absolute and relative (in %) uncertainties
     */
    double Result::abs_to_rel(const double v, const double e) {
        return 100 * e / v;
    };

    double Result::rel_to_abs(const double v, const double e) {
        return v * e / 100;
    };

    // DEBUG: need to implement properly
    val::BaseValue::PointerType Result::abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
        return e->math_div(v.get())->math_mul(std::make_unique<val::ArrayValueFloat64>(100).get());
    };

    val::BaseValue::PointerType Result::rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
        return v->math_mul(e.get())->math_div(std::make_unique<val::ArrayValueFloat64>(100).get());
    };

    /*
     * Return a string representation of a measurement result
     */
    std::string Result::to_string(const UnitFormat& format) const {
        std::stringstream ss;
        core::StringFormatType fmt;
        fmt.valuePrecision = format.precision;
        if (uncertainty == nullptr || !format.display_uncertainty()) {
            ss << estimate->to_string(fmt);
        } else {
            val::ArrayValueFloat64 dvalue(estimate.get());
            val::ArrayValueFloat64 duncertainty(uncertainty.get());
            ss << core::array_to_string(dvalue.get_values(), duncertainty.get_values(), dvalue.get_shape(), fmt);
        }
        return format.format_order(ss.str());
    }

    std::size_t Result::size() const {
        return estimate->get_size();
        // return estimate.size();
    }

    val::Array::ShapeType Result::shape() const {
        return estimate->get_shape();
    }

    /*
     * Add two measurement results
     */
    Result operator+(const Result& m1, const Result& m2) {
        if (Result::propagation_mode() == Result::PropagationMode::Interval) {
            auto error = m1.uncertainty && m2.uncertainty
                             ? m1.uncertainty->math_add(m2.uncertainty.get())
                             : (m1.uncertainty ? m1.uncertainty->clone()
                                               : (m2.uncertainty ? m2.uncertainty->clone() : nullptr));
            return Result(m1.estimate->math_add(m2.estimate.get()), std::move(error));
        }
        m1.ensure_source();
        m2.ensure_source();
        Result out(m1.estimate->math_add(m2.estimate.get()));
        add_scaled_sources(out.sources, m1.sources, nullptr);
        add_scaled_sources(out.sources, m2.sources, nullptr);
        out.refresh_uncertainty();
        return out;
    }

    void Result::operator+=(const Result& m) {
        Result out = *this + m;
        estimate = std::move(out.estimate);
        uncertainty = std::move(out.uncertainty);
        sources = std::move(out.sources);
        // estimate += m.estimate;
        // uncertainty += m.uncertainty;
        //// uncertainty = math::sqrt(math::pow(uncertainty,2)+math::pow(m.uncertainty,2));
    }

    /*
     * Subtract two measurement results
     */
    Result operator-(const Result& m1) {
        auto minus_one = std::make_unique<val::ArrayValueFloat64>(-1.0);
        return m1.transformed(m1.estimate->math_neg(), std::move(minus_one));
        // return Result(-m1.estimate, m1.uncertainty);
    }
    Result operator-(const Result& m1, const Result& m2) {
        if (Result::propagation_mode() == Result::PropagationMode::Interval) {
            auto error = m1.uncertainty && m2.uncertainty
                             ? m1.uncertainty->math_add(m2.uncertainty.get())
                             : (m1.uncertainty ? m1.uncertainty->clone()
                                               : (m2.uncertainty ? m2.uncertainty->clone() : nullptr));
            return Result(m1.estimate->math_sub(m2.estimate.get()), std::move(error));
        }
        m1.ensure_source();
        m2.ensure_source();
        Result out(m1.estimate->math_sub(m2.estimate.get()));
        auto negative = std::make_unique<val::ArrayValueFloat64>(-1.0);
        add_scaled_sources(out.sources, m1.sources, nullptr);
        add_scaled_sources(out.sources, m2.sources, negative.get());
        out.refresh_uncertainty();
        return out;
        // return Result(m1.estimate - m2.estimate, m1.uncertainty + m2.uncertainty);
    }

    void Result::operator-=(const Result& m) {
        Result out = *this - m;
        estimate = std::move(out.estimate);
        uncertainty = std::move(out.uncertainty);
        sources = std::move(out.sources);
        // estimate -= m.estimate;
        // uncertainty += m.uncertainty;
    }

    /*
     * Multiply measurement result by another measurement result
     */
    const Result multiply(const Result* m, const Result* n) {
        if (Result::propagation_mode() == Result::PropagationMode::Interval) {
            Result output(m->estimate->math_mul(n->estimate.get()));
            if (m->uncertainty && n->uncertainty) {
                auto upper = m->estimate->math_add(m->uncertainty.get())
                                 ->math_mul(n->estimate->math_add(n->uncertainty.get()).get())
                                 ->math_sub(output.estimate.get())->math_abs();
                auto lower = m->estimate->math_sub(m->uncertainty.get())
                                 ->math_mul(n->estimate->math_sub(n->uncertainty.get()).get())
                                 ->math_sub(output.estimate.get())->math_abs();
                output.uncertainty = upper->math_max(lower.get());
            } else if (m->uncertainty) {
                output.uncertainty = m->uncertainty->math_mul(n->estimate.get());
            } else if (n->uncertainty) {
                output.uncertainty = n->uncertainty->math_mul(m->estimate.get());
            }
            return output;
        }
        m->ensure_source();
        n->ensure_source();
        Result nm(m->estimate->math_mul(n->estimate.get()));
        add_scaled_sources(nm.sources, m->sources, n->estimate.get());
        add_scaled_sources(nm.sources, n->sources, m->estimate.get());
        nm.refresh_uncertainty();
        return nm;
    }

    Result operator*(const Result& m1, const Result& m2) {
        return multiply(&m1, &m2);
    }

    void Result::operator*=(const Result& m) {
        Result nm = multiply(this, &m);
        estimate = nm.estimate->clone();
        uncertainty = nm.uncertainty ? nm.uncertainty->clone() : nullptr;
        sources = std::move(nm.sources);
        // estimate = nm.estimate;
        // uncertainty = nm.uncertainty;
    }

    /*
     * Divide measurement result by another measurement result
     */
    const Result divide(const Result* m, const Result* n) {
        Result nm(m->estimate->math_div(n->estimate.get()));
        if (Result::propagation_mode() == Result::PropagationMode::Interval) {
            if (m->uncertainty && n->uncertainty) {
                auto upper = m->estimate->math_add(m->uncertainty.get())
                                 ->math_div(n->estimate->math_sub(n->uncertainty.get()).get())
                                 ->math_sub(nm.estimate.get())->math_abs();
                auto lower = m->estimate->math_sub(m->uncertainty.get())
                                 ->math_div(n->estimate->math_add(n->uncertainty.get()).get())
                                 ->math_sub(nm.estimate.get())->math_abs();
                nm.uncertainty = upper->math_max(lower.get());
            } else if (m->uncertainty) {
                nm.uncertainty = m->uncertainty->math_div(n->estimate.get());
            } else if (n->uncertainty) {
                auto upper = m->estimate->math_div(n->estimate->math_add(n->uncertainty.get()).get())
                                 ->math_sub(nm.estimate.get())->math_abs();
                auto lower = m->estimate->math_div(n->estimate->math_sub(n->uncertainty.get()).get())
                                 ->math_sub(nm.estimate.get())->math_abs();
                nm.uncertainty = upper->math_max(lower.get());
            }
            return nm;
        }
        m->ensure_source();
        n->ensure_source();
        auto numerator_derivative = n->estimate->math_inv();
        auto denominator_squared = n->estimate->math_pow(2);
        auto denominator_derivative = m->estimate->math_neg()->math_div(denominator_squared.get());
        add_scaled_sources(nm.sources, m->sources, numerator_derivative.get());
        add_scaled_sources(nm.sources, n->sources, denominator_derivative.get());
        nm.refresh_uncertainty();
        // Result nm(m->estimate / n->estimate);
        // if (m->uncertainty == 0 && n->uncertainty == 0) {
        //   nm.uncertainty = (m->uncertainty.size() > n->uncertainty.size()) ? m->uncertainty : n->uncertainty;
        // } else if (m->uncertainty == 0 && n->uncertainty != 0) {
        //   val::BaseValue::PointerType maxuncertainty = math::abs(m->estimate / (n->estimate + n->uncertainty) -
        //   nm.estimate); val::BaseValue::PointerType minuncertainty = math::abs(m->estimate / (n->estimate -
        //   n->uncertainty) - nm.estimate); nm.uncertainty = math::max(maxuncertainty, minuncertainty);
        // } else if (m->uncertainty != 0 && n->uncertainty == 0) {
        //   nm.uncertainty = m->uncertainty / n->estimate;
        // } else {
        //   val::BaseValue::PointerType maxuncertainty = math::abs((m->estimate + m->uncertainty) / (n->estimate -
        //   n->uncertainty) - nm.estimate); val::BaseValue::PointerType minuncertainty = math::abs((m->estimate -
        //   m->uncertainty) / (n->estimate + n->uncertainty) - nm.estimate); nm.uncertainty = math::max(maxuncertainty,
        //   minuncertainty);
        // }
        return nm;
    }

    Result operator/(const Result& m1, const Result& m2) {
        return divide(&m1, &m2);
    }

    void Result::operator/=(const Result& m) {
        Result nm = divide(this, &m);
        estimate = nm.estimate->clone();
        uncertainty = nm.uncertainty ? nm.uncertainty->clone() : nullptr;
        sources = std::move(nm.sources);
        // estimate = nm.estimate;
        // uncertainty = nm.uncertainty;
    }

    void Result::pow(const ExponentVariant& e) {
        ensure_source();
        double fexp = exponent_to_float(e);
        auto derivative = estimate->math_pow(fexp - 1)->math_mul(std::make_unique<val::ArrayValueFloat64>(fexp).get());
        estimate = estimate->math_pow(fexp);
        for (auto& [id, contribution] : sources)
            contribution = contribution->math_mul(derivative.get());
        refresh_uncertainty();
    }

    bool Result::operator==(const Result& a) const {
        if (uncertainty && a.uncertainty)
            return (estimate->compare_equal(a.estimate.get())->all_of()) &&
                   (uncertainty->compare_equal(a.uncertainty.get())->all_of());
        else if (!uncertainty && !a.uncertainty)
            return (estimate->compare_equal(a.estimate.get())->all_of());
        else
            return false;
        // return (estimate == a.estimate) && (uncertainty == a.uncertainty);
    };

    bool Result::operator!=(const Result& a) const {
        if (uncertainty && a.uncertainty)
            return (estimate->compare_not_equal(a.estimate.get())->any_of()) ||
                   (uncertainty->compare_not_equal(a.uncertainty.get())->any_of());
        else if (!uncertainty && !a.uncertainty)
            return (estimate->compare_not_equal(a.estimate.get())->any_of());
        else
            return false;
        // return (estimate != a.estimate) || (uncertainty != a.uncertainty);
    };

    std::ostream& operator<<(std::ostream& os, const Result& m) {
        os << m.to_string();
        return os;
    }

} // namespace snt::puq
