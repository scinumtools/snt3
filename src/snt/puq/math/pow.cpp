#include <snt/puq/base_units.h>
#include <snt/puq/exceptions.h>
#include <snt/puq/exponent.h>
#include <snt/puq/math/pow.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace snt::puq::math {

    namespace {

        Exponent rational_exponent(const double value) {
            if (!std::isfinite(value) || std::abs(value) > std::numeric_limits<int>::max())
                throw puq::UnitException(
                    "Invalid unit exponent",
                    "The power exponent cannot be represented as a rational unit exponent.",
                    "Use a finite exponent within the supported unit-exponent range.",
                    __FILE__,
                    __LINE__
                );

            constexpr int max_denominator = 100000;
            const double tolerance = 1e-12 * std::max(1.0, std::abs(value));
            for (int denominator = 1; denominator <= max_denominator; ++denominator) {
                const auto numerator = std::llround(value * denominator);
                if (std::abs(value - static_cast<double>(numerator) / denominator) <= tolerance &&
                    std::abs(numerator) <= std::numeric_limits<int>::max())
                    return Exponent(static_cast<int>(numerator), denominator);
            }
            throw puq::UnitException(
                "Unsupported unit exponent",
                "The power exponent cannot be represented by a rational denominator up to 100000.",
                "Use a rational exponent such as 0.5 or 1/3 for quantities with units.",
                __FILE__,
                __LINE__
            );
        }

    } // namespace

    puq::BaseUnits pow(puq::BaseUnits bu, const ExponentVariant& exp) {
        for (auto& unit : bu.baseunits) {
            unit.exponent =
                std::visit([](auto const& a, auto const& b) -> ExponentVariant { return a * b; }, unit.exponent, exp);
        }
        return bu;
    }

    puq::Result pow(const puq::Result& res, const double exp) {
        // z ± Dz = pow(x ± Dx, y) -> Dz = y * pow(x, y-1) * Dx
        if (res.uncertainty) {
            std::unique_ptr<val::ArrayValueFloat64> cst = std::make_unique<val::ArrayValueFloat64>(exp);
            return puq::Result(
                res.estimate->math_pow(exp),
                res.estimate->math_pow(exp - 1)->math_mul(cst.get())->math_abs()->math_mul(res.uncertainty.get())
            );
        } else {
            return puq::Result(res.estimate->math_pow(exp));
        }
        // return puq::Result(pow(res.estimate, exp), abs(exp * pow(res.estimate, exp - 1)) * res.uncertainty);
    }

    puq::Result pow(const puq::Result& res1, const puq::Result& res2) {
        // Dz = sqrt(pow(Dzx,2)+pow(Dzy,2))
        // z ± Dz = pow(x ± Dx, y ± Dy)
        std::unique_ptr<val::ArrayValueFloat64> cst = std::make_unique<val::ArrayValueFloat64>(-1);
        if (res1.uncertainty && res2.uncertainty) {
            val::BaseValue::PointerType Dzx =
                res2.estimate->math_mul(res1.estimate->math_pow(res2.estimate->math_add(cst.get()).get()).get())
                    ->math_mul(res1.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
            val::BaseValue::PointerType Dzy = res1.estimate->math_pow(res2.estimate.get())
                                                  ->math_mul(res1.estimate->math_log().get())
                                                  ->math_mul(res2.uncertainty.get()); // Dzy = pow(x, y) * log(x) * Dy
            return puq::Result(
                res1.estimate->math_pow(res2.estimate.get()),
                Dzx->math_pow(2)->math_add(Dzy->math_pow(2).get())->math_sqrt()
            );
        } else if (res1.uncertainty) {
            val::BaseValue::PointerType Dzx =
                res2.estimate->math_mul(res1.estimate->math_pow(res2.estimate->math_add(cst.get()).get()).get())
                    ->math_mul(res1.uncertainty.get()); // Dzx = y * pow(x, y-1) * Dx
            return puq::Result(res1.estimate->math_pow(res2.estimate.get()), Dzx->math_pow(2)->math_sqrt());
        } else if (res2.uncertainty) {
            val::BaseValue::PointerType Dzy = res1.estimate->math_pow(res2.estimate.get())
                                                  ->math_mul(res1.estimate->math_log().get())
                                                  ->math_mul(res2.uncertainty.get()); // Dzy = pow(x, y) * log(x) * Dy
            return puq::Result(res1.estimate->math_pow(res2.estimate.get()), Dzy->math_pow(2)->math_sqrt());
        } else {
            return puq::Result(res1.estimate->math_pow(res2.estimate.get()));
        }
        // val::BaseValue::PointerType Dzx = res2.estimate * pow(res1.estimate, res2.estimate - 1) * res1.uncertainty;
        // // Dzx = y * pow(x, y-1) * Dx val::BaseValue::PointerType Dzy = pow(res1.estimate, res2.estimate) *
        // log(res1.estimate) * res2.uncertainty; // Dzy = pow(x, y) * log(x) * Dy return puq::Result(pow(res1.estimate,
        // res2.estimate), sqrt(Dzx * Dzx + Dzy * Dzy));
    }

    puq::Measurement pow(const puq::Measurement& msr, const double exp) {
        const auto units = msr.baseunits.has_dimensions() ? pow(msr.baseunits, rational_exponent(exp)) : msr.baseunits;
        return puq::Measurement(pow(msr.result, exp), units);
    }

    puq::Measurement pow(const puq::Measurement& msr, const ExponentVariant& exp) {
        // WARNING: if exp is not converted in float in the pow(msr.result, ..)
        //          the exponent will be casted as a Measurement and calculation
        //          will loop infinitely
        return puq::Measurement(pow(msr.result, exponent_to_float(exp)), pow(msr.baseunits, exp));
    }

    puq::Measurement pow(const puq::Measurement& msr1, const puq::Measurement& msr2) {
        if (msr2.baseunits.has_dimensions())
            throw puq::UnitException(
                "Dimension mismatch",
                "The exponent in the power function must be dimensionless.",
                "Provide a dimensionless quantity as the exponent.",
                __FILE__,
                __LINE__
            );
        return puq::Measurement(pow(msr1.result, msr2.result), msr1.baseunits);
    }

    puq::Quantity pow(const puq::Quantity& quant, const double exp) {
        return puq::Quantity(pow(quant.measurement, exp), quant.stype);
    }

    puq::Quantity pow(const puq::Quantity& quant1, const puq::Quantity& quant2) {
        return puq::Quantity(pow(quant1.measurement, quant2.measurement), quant1.stype);
    }

} // namespace snt::puq::math
