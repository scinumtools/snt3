#ifndef PUQ_RESULT_H
#define PUQ_RESULT_H

#include <snt/puq/exponent.h>
#include <snt/puq/settings.h>
#include <snt/puq/unit_format.h>
#include <snt/val/values.h>
#include <variant>

namespace snt::puq {

    /** Variant accepted for scalar or polymorphic value storage. */
    using ValueVariant = std::variant<double, val::BaseValue::PointerType>;

    /** Numerical estimate with an optional absolute uncertainty. */
    class Result {
      public:
        val::BaseValue::PointerType estimate;    ///< Estimated value
        val::BaseValue::PointerType uncertainty; ///< Optional absolute uncertainty
        Result(const Result& other) {
            estimate = other.estimate ? other.estimate->clone() : nullptr;
            uncertainty = other.uncertainty ? other.uncertainty->clone() : nullptr;
        }
        Result& operator=(const Result& other) {
            if (this != &other) {
                estimate = other.estimate ? other.estimate->clone() : nullptr;
                uncertainty = other.uncertainty ? other.uncertainty->clone() : nullptr;
            }
            return *this;
        }
        Result(Result&&) noexcept = default;
        Result& operator=(Result&&) noexcept = default;
        Result() : estimate(std::make_unique<val::ArrayValueFloat64>(1)), uncertainty(nullptr) {}
        Result(const double m) : estimate(std::make_unique<val::ArrayValueFloat64>(m)), uncertainty(nullptr) {}
        Result(const double m, const double e)
            : estimate(std::make_unique<val::ArrayValueFloat64>(m)),
              uncertainty(std::make_unique<val::ArrayValueFloat64>(e)) {}
        Result(val::BaseValue::PointerType m);
        Result(val::BaseValue::PointerType m, val::BaseValue::PointerType e);

        static double abs_to_rel(const double v, const double a);
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
        void operator+=(const Result& m);
        void operator-=(const Result& m);
        void operator*=(const Result& m);
        void operator/=(const Result& m);
        bool operator==(const Result& a) const;
        bool operator!=(const Result& a) const;
        /** Raise the result to an exponent. */
        void pow(const ExponentVariant& e);
    };

} // namespace snt::puq

#endif // PUQ_RESULT_H
