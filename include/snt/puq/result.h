#ifndef PUQ_RESULT_H
#define PUQ_RESULT_H

#include <snt/puq/exponent.h>
#include <snt/puq/settings.h>
#include <snt/puq/unit_format.h>
#include <snt/val/values.h>
#include <variant>
#include <map>
#include <vector>

namespace snt::puq {

    /** Variant accepted for scalar or polymorphic value storage. */
    using ValueVariant = std::variant<double, val::BaseValue::PointerType>;

    /** Numerical estimate with an optional absolute uncertainty.
     *
     * Values may be scalars or VAL arrays. When present, `uncertainty` is an
     * absolute uncertainty in the same units and shape as `estimate`.
     */
    class Result {
      public:
        enum class PropagationMode { Standard, Interval };
        using SourceMap = std::map<std::size_t, val::BaseValue::PointerType>;
        val::BaseValue::PointerType estimate;    ///< Estimated scalar or array value.
        val::BaseValue::PointerType uncertainty; ///< Optional absolute uncertainty matching `estimate`.
        /** Signed first-order contributions, keyed by independent source identity. */
        SourceMap sources;
        Result(const Result& other) {
            estimate = other.estimate ? other.estimate->clone() : nullptr;
            uncertainty = other.uncertainty ? other.uncertainty->clone() : nullptr;
            for (const auto& [id, contribution] : other.sources)
                sources.emplace(id, contribution->clone());
        }
        /** Assign the right-hand result to this object.
         * @param other Other value used in the operation.
         */
        Result& operator=(const Result& other) {
            if (this != &other) {
                estimate = other.estimate ? other.estimate->clone() : nullptr;
                uncertainty = other.uncertainty ? other.uncertainty->clone() : nullptr;
                sources.clear();
                for (const auto& [id, contribution] : other.sources)
                    sources.emplace(id, contribution->clone());
            }
            return *this;
        }
        Result(Result&&) noexcept = default;
        Result& operator=(Result&&) noexcept = default;
        Result() : estimate(std::make_unique<val::ArrayValueFloat64>(1)), uncertainty(nullptr) {}
        Result(const double m) : estimate(std::make_unique<val::ArrayValueFloat64>(m)), uncertainty(nullptr) {}
        Result(const double m, const double e);
        Result(val::BaseValue::PointerType m);
        Result(val::BaseValue::PointerType m, val::BaseValue::PointerType e);
        /** Apply a pointwise derivative while retaining the input source identities. */
        Result transformed(val::BaseValue::PointerType value, val::BaseValue::PointerType derivative) const;
        /** Recompute the displayed standard uncertainty from source contributions. */
        void refresh_uncertainty();
        /** Set the correlation coefficient between two primitive uncertain results.
         * Both results must each contain one uncertainty source and have matching shapes.
         */
        static void correlate(Result& left, Result& right, double coefficient);
        /** Assign a covariance matrix to a set of primitive uncertain results. */
        static void correlate_many(const std::vector<Result*>& values, const std::vector<std::vector<double>>& covariance);
        /** Select process-wide propagation semantics. Standard is the default. */
        static void set_propagation_mode(PropagationMode mode);
        static PropagationMode propagation_mode();

        /** Convert an absolute uncertainty to a relative uncertainty.
         * @param v Central value.
         * @param a Absolute uncertainty matching `v`.
         */
        static double abs_to_rel(const double v, const double a);
        /** Convert a relative uncertainty to an absolute uncertainty.
         * @param v Central value.
         * @param r Relative uncertainty expressed as a fraction of `v`.
         */
        static double rel_to_abs(const double v, const double r);
        static val::BaseValue::PointerType abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType a);
        static val::BaseValue::PointerType rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType r);

        /** Return the number of stored estimates. */
        std::size_t size() const;
        /** Return the estimate array shape. */
        val::Array::ShapeType shape() const;
        /** Format the estimate and uncertainty. */
        std::string to_string(const UnitFormat& format = UnitFormat()) const;
        friend Result operator-(const Result& m1);
        friend Result operator+(const Result& m1, const Result& m2);
        friend Result operator-(const Result& m1, const Result& m2);
        friend Result operator*(const Result& m1, const Result& m2);
        friend Result operator/(const Result& m1, const Result& m2);
        friend std::ostream& operator<<(std::ostream& os, const Result& m);
        /** Add the right-hand result in place and propagate its uncertainty.
         * @param m Result with a compatible shape.
         */
        void operator+=(const Result& m);
        /** Subtract the right-hand result in place and propagate its uncertainty.
         * @param m Result with a compatible shape.
         */
        void operator-=(const Result& m);
        /** Multiply by the right-hand result in place and propagate its uncertainty.
         * @param m Result with a compatible shape.
         */
        void operator*=(const Result& m);
        /** Divide by the right-hand result in place and propagate its uncertainty.
         * @param m Result with a compatible shape.
         */
        void operator/=(const Result& m);
        /** Compare the two values for equality.
         * @param a First operand.
         */
        bool operator==(const Result& a) const;
        /** Compare the two values for inequality.
         * @param a First operand.
         */
        bool operator!=(const Result& a) const;
        /** Raise the result to an exponent. */
        void pow(const ExponentVariant& e);
    };

} // namespace snt::puq

#endif // PUQ_RESULT_H
