#include <algorithm>
#include <iostream>
#include <set>
#include <snt/puq/converter.h>
#include <snt/puq/exceptions.h>
#include <snt/puq/math/exp.h>
#include <snt/puq/math/log.h>
#include <snt/puq/math/log10.h>
#include <snt/puq/math/pow.h>

namespace snt::puq {

    namespace {

        constexpr double nine_fifths = 9.0 / 5.0;
        constexpr double five_ninths = 5.0 / 9.0;

        bool get_exact_scalar(const Result& result, double& value) {
            if (!result.estimate || result.estimate->get_size() != 1)
                return false;
            if (result.uncertainty && result.uncertainty->any_of())
                return false;
            const auto* scalar = dynamic_cast<const val::ArrayValue<double>*>(result.estimate.get());
            if (!scalar)
                return false;
            value = scalar->get_value(0);
            return true;
        }

        template <typename Transform>
        void write_temperature_expression(
            const val::ArrayValue<double>& array,
            double source_scale,
            double target_scale,
            Transform transform,
            Result& output
        ) {
            auto source = val::array_expression(array.get_data(), array.get_size(), array.get_shape());
            auto scaled = val::map_array_expression(
                std::move(source), [source_scale](double value) { return value * source_scale; }
            );
            auto adjusted = val::map_array_expression(std::move(scaled), std::move(transform));
            auto converted = val::map_array_expression(
                std::move(adjusted), [target_scale](double value) { return value / target_scale; }
            );
            auto values = val::evaluate_array_expression(converted);
            output = Result(std::make_unique<val::ArrayValueFloat64>(std::move(values), array.get_shape()));
        }

        bool try_temperature_float64_expression(
            const Result& input,
            const std::string& source_unit,
            const std::string& target_unit,
            double source_scale,
            double target_scale,
            Result& output
        ) {
            if (input.uncertainty || target_scale == 0.0)
                return false;
            const auto* array = dynamic_cast<const val::ArrayValue<double>*>(input.estimate.get());
            if (!array || array->get_dtype() != core::DataType::Float64 || array->get_size() <= 1)
                return false;

            if (source_unit == "K" && target_unit == "Cel")
                write_temperature_expression(*array, source_scale, target_scale, [](double value) { return value - 273.15; }, output);
            else if (source_unit == "K" && target_unit == "degF")
                write_temperature_expression(
                    *array, source_scale, target_scale,
                    [](double value) { return (value - 273.15) * nine_fifths + 32.0; }, output
                );
            else if (source_unit == "degR" && target_unit == "degF")
                write_temperature_expression(
                    *array, source_scale, target_scale, [](double value) { return value * nine_fifths - 459.67; }, output
                );
            else if (source_unit == "degR" && target_unit == "Cel")
                write_temperature_expression(
                    *array, source_scale, target_scale,
                    [](double value) { return (value * nine_fifths - 491.67) * five_ninths; }, output
                );
            else if (source_unit == "Cel" && target_unit == "K")
                write_temperature_expression(*array, source_scale, target_scale, [](double value) { return value + 273.15; }, output);
            else if (source_unit == "Cel" && target_unit == "degF")
                write_temperature_expression(
                    *array, source_scale, target_scale, [](double value) { return value * nine_fifths + 32.0; }, output
                );
            else if (source_unit == "Cel" && target_unit == "degR")
                write_temperature_expression(
                    *array, source_scale, target_scale,
                    [](double value) { return (value * nine_fifths + 491.67) * five_ninths; }, output
                );
            else if (source_unit == "degF" && target_unit == "K")
                write_temperature_expression(
                    *array, source_scale, target_scale,
                    [](double value) { return ((value - 32.0) * five_ninths) + 273.15; }, output
                );
            else if (source_unit == "degF" && target_unit == "Cel")
                write_temperature_expression(
                    *array, source_scale, target_scale, [](double value) { return (value - 32.0) * five_ninths; }, output
                );
            else if (source_unit == "degF" && target_unit == "degR")
                write_temperature_expression(
                    *array, source_scale, target_scale, [](double value) { return (value + 459.67) * five_ninths; }, output
                );
            else
                return false;
            return true;
        }

        bool scale_owned_array(val::BaseValue::PointerType& value, double source_scale, double target_scale) {
            auto* array = dynamic_cast<val::ArrayValue<double>*>(value.get());
            if (!array)
                return false;
            if (source_scale == target_scale)
                return true;

            // Keep the original multiply-then-divide order for numerical
            // stability, but perform both operations in one output-buffer pass.
            double* data = array->get_data();
            for (std::size_t i = 0; i < array->get_size(); ++i)
                data[i] = (data[i] * source_scale) / target_scale;
            return true;
        }

        bool make_scaled_float64_copy(
            const val::BaseValue* source,
            double source_scale,
            double target_scale,
            val::BaseValue::PointerType& output
        ) {
            const auto* array = dynamic_cast<const val::ArrayValue<double>*>(source);
            if (!array || array->get_dtype() != core::DataType::Float64)
                return false;

            const std::size_t size = array->get_size();
            const double* source_values = array->get_data();
            val::ArrayStorage<double> values(size);
            if (size > 0) {
                if (source_scale == target_scale) {
                    std::copy_n(source_values, size, values.begin());
                } else {
                    for (std::size_t i = 0; i < size; ++i)
                        values[i] = (source_values[i] * source_scale) / target_scale;
                }
            }
            output = std::make_unique<val::ArrayValueFloat64>(std::move(values), array->get_shape());
            return true;
        }

    } // namespace

    Result Converter::_convert_linear(const Result& m1, const Result& m2) {
        // The common unit conversion has exact scalar scale factors and no
        // contextual multiplier. Copy and scale Float64 data into the result in
        // one pass, preserving ownership without an intermediate full-array copy.
        double source_scale = 0.0;
        double target_scale = 0.0;
        double context_scale = 0.0;
        if (get_exact_scalar(dimensions1.numerical, source_scale) &&
            get_exact_scalar(dimensions2.numerical, target_scale) &&
            get_exact_scalar(m2, context_scale) && context_scale == 1.0 && target_scale != 0.0) {
            val::BaseValue::PointerType estimate;
            if (!make_scaled_float64_copy(m1.estimate.get(), source_scale, target_scale, estimate)) {
                estimate = m1.estimate->clone();
                if (!scale_owned_array(estimate, source_scale, target_scale))
                    return (m1 * dimensions1.numerical) / (m2 * dimensions2.numerical);
            }

            auto scale = std::make_unique<val::ArrayValueFloat64>(source_scale / target_scale);
            return m1.transformed(std::move(estimate), std::move(scale));
        }

        // Keep the general arithmetic path for contextual or non-scalar scales.
        return (m1 * dimensions1.numerical) / (m2 * dimensions2.numerical);
    }

    inline Result _convert_B_B(const Result& value, const Result& exp = 0) {
        return value + exp;
    }

    inline Result _convert_B_Np(const Result& value) {
        return (Result)1.151277918 * value;
    }

    inline Result _convert_Np_B(const Result& value) {
        return value / (Result)1.151277918;
    }

    inline Result _convert_Ratio_B(const Result& value, const Result& exp) {
        return exp * math::log10(value);
    }

    inline Result _convert_B_Ratio(const Result& value, const Result& exp) {
        return math::pow((Result)10, value / exp);
    }

    inline Result _convert_Ratio_Np(const Result& value, const Result& exp) {
        return exp * math::log(value);
    }

    inline Result _convert_Np_Ratio(const Result& value, const Result& exp) {
        return math::exp(value / exp);
    }

    inline Result _convert_Mw_Mo(const Result& value) {
        return math::pow((Result)10, 1.5 * (value + (Result)10.7));
    }

    inline Result _convert_Mo_Mw(const Result& value) {
        return 2. / 3. * math::log10(value) - (Result)10.7;
    }

    Result Converter::_convert_logarithmic(const Result& m) {
        const std::string s1 = baseunits1[0].unit;
        const std::string s2 = baseunits2[0].unit;
        const Result n1 = dimensions1.numerical;
        const Result n2 = dimensions2.numerical;
        if (s1 == "Np") { // Nepers -> Bel or Ratio
            if (s2 == "B")
                return _convert_Np_B(m * n1) / n2;
            else if (s2 == "AR")
                return _convert_Np_Ratio(m * n1, 1) / n2;
            else if (s2 == "PR")
                return _convert_Np_Ratio(m * n1, 0.5) / n2;
        } else if (s2 == "Np") { // Bel or Ratio -> Nepers
            if (s1 == "B")
                return _convert_B_Np(m * n1) / n2;
            else if (s1 == "AR")
                return _convert_Ratio_Np(m * n1, 1) / n2;
            else if (s1 == "PR")
                return _convert_Ratio_Np(m * n1, 0.5) / n2;
        } else if (s1 == "Mw" && (s2 == "Mo" || s2 == "dyn")) { // moment magnitude -> seismic moment
            return _convert_Mw_Mo(m * n1) / (n2 * 1e4);         // 1e4 to MGS
        } else if ((s1 == "Mo" || s1 == "dyn") && s2 == "Mw") { // seismic moment -> moment magnitude
            return _convert_Mo_Mw(m * n1 * 1e4) / n2;           // 1e4 to MGS
        } else {
            struct pair {
                std::string first;
                std::string second;
                double exp;
                double conv;
            };
            const std::vector<pair> pairs_B_B = {
                {"BW", "Bm", 3},
                {"BW", "BmW", 3},
                {"Bm", "BmW", 0},
                {"BV", "BuV", 12},
            };
            for (auto& pair : pairs_B_B) {
                if (s1 == pair.first && s2 == pair.second) { // BelX -> BelY
                    return _convert_B_B(m * n1, pair.exp) / n2;
                } else if (s1 == pair.second && s2 == pair.first) { // BelY -> BelX
                    return _convert_B_B(m * n1, -pair.exp) / n2;
                }
            }
            const std::vector<pair> pairs_B_Ratio = {
                {"B", "PR", 1, 1},
                {"Bm", "W", 1, 1},
                {"BmW", "W", 1, 1},
                {"BW", "W", 1, 1e-3},
                {"BSIL", "W", 1, 1e9},
                {"BSWL", "W", 1, 1e9},
                {"B", "AR", 2, 1},
                {"BV", "V", 2, 1e-3},
                {"BuV", "V", 2, 1e3},
                {"BA", "A", 2, 1},
                {"BuA", "A", 2, 1e6},
                {"BOhm", "Ohm", 2, 1e-3},
                {"BSPL", "Pa", 2, 50},
            };
            for (auto& pair : pairs_B_Ratio) {
                if (s1 == pair.first && s1 == s2) { // Bel -> Bel
                    return _convert_B_B(m * n1) / n2;
                } else if (s1 == pair.first && s2 == pair.second) { // Bel -> Ratio
                    return _convert_B_Ratio(m * n1, pair.exp) / (n2 * pair.conv);
                } else if (s1 == pair.second && s2 == pair.first) { // Ratio -> Bel
                    return _convert_Ratio_B(m * n1 * pair.conv, pair.exp) / n2;
                }
            }
        }
        throw puq::UnitException(
            "Conversion unavailable",
            "The conversion from `" + s1 + "` to `" + s2 + "` is not available.",
            "Use a supported conversion between compatible units.",
            __FILE__,
            __LINE__
        );
    }

    Result Converter::_convert_temperature(const Result& m) {
        std::string s1 = baseunits1[0].unit;
        std::string s2 = baseunits2[0].unit;
        Result const9div5 = 9. / 5.;
        Result const5div9 = 5. / 9.;
        if (s1 == s2) {
            return m;
        }

        double source_scale = 0.0;
        double target_scale = 0.0;
        Result fused;
        if (get_exact_scalar(dimensions1.numerical, source_scale) &&
            get_exact_scalar(dimensions2.numerical, target_scale) &&
            try_temperature_float64_expression(m, s1, s2, source_scale, target_scale, fused))
            return fused;

        // Keep the existing Result operations for scalars, uncertainty-bearing
        // values, and storage types that cannot use the elementwise expression.
        Result converted = m * dimensions1.numerical;
        if (s1 == "K" && s2 == "Cel") {
            converted = converted - (Result)273.15;
        } else if (s1 == "K" && s2 == "degF") {
            converted = (converted - (Result)273.15) * const9div5 + (Result)32;
        } else if (s1 == "degR" && s2 == "degF") {
            converted = converted * const9div5 - (Result)459.67;
        } else if (s1 == "degR" && s2 == "Cel") {
            converted = (converted * const9div5 - (Result)491.67) * const5div9;
        } else if (s1 == "Cel" && s2 == "K") {
            converted = converted + (Result)273.15;
        } else if (s1 == "Cel" && s2 == "degF") {
            converted = (converted * const9div5) + (Result)32;
        } else if (s1 == "Cel" && s2 == "degR") {
            converted = ((converted * const9div5) + (Result)491.67) * const5div9;
        } else if (s1 == "degF" && s2 == "K") {
            converted = ((converted - (Result)32) * const5div9) + (Result)273.15;
        } else if (s1 == "degF" && s2 == "Cel") {
            converted = (converted - (Result)32) * const5div9;
        } else if (s1 == "degF" && s2 == "degR") {
            converted = (converted + (Result)459.67) * const5div9;
        } else {
            throw puq::UnitException(
                "Conversion unavailable",
                "The conversion from `" + s1 + "` to `" + s2 + "` is not available.",
                "Use a supported conversion between compatible units.",
                __FILE__,
                __LINE__
            );
        }
        return converted / dimensions2.numerical;
    }

    Converter::Converter(const BaseUnits& bu1, const BaseUnits& bu2)
        : baseunits1(bu1), baseunits2(bu2), utype(Utype::NUL) {
        // determine dimensions
        dimensions1 = bu1.dimensions();
        dimensions2 = bu2.dimensions();

        // make sure that physical dimensions are matching
        for (int i = 0; i < Config::num_basedim; i++) {
            if (!equal_exp(dimensions1.physical[i], dimensions2.physical[i]))
                throw puq::ConverterException(bu1, bu2, __FILE__, __LINE__);
        }

        // determine conversion type
        // std::cout << baseunits1.to_string() << " " << baseunits2.to_string() << '\n';
        // std::cout << std::bitset<8>((int)dimensions1.utype) << '\n';
        // std::cout << std::bitset<8>((int)dimensions2.utype) << '\n';
        if constexpr (Config::use_units_logarithmic) {
            if (((dimensions1.utype | dimensions2.utype) & Utype::LOG) == Utype::LOG) {
                if ((baseunits1.size() == 1 || baseunits1.size() == 2) &&
                    (baseunits2.size() == 1 || baseunits2.size() == 2)) {
                    utype = Utype::LOG;
                    goto finalize_init;
                }
            }
        }
        if constexpr (Config::use_units_temperature) {
            if (((dimensions1.utype | dimensions2.utype) & Utype::TMP) == Utype::TMP) {
                if ((baseunits1.size() == 1 || baseunits1.size() == 2) &&
                    (baseunits2.size() == 1 || baseunits2.size() == 2)) {
                    utype = Utype::TMP;
                    goto finalize_init;
                }
            }
        }
        if (((dimensions1.utype & dimensions2.utype) & Utype::LIN) == Utype::LIN) {
            utype = Utype::LIN;
        }
    finalize_init:;
    };

    Result Converter::convert(const Result& m1, const Result& m2) {
        if constexpr (Config::debug_converter) {
            std::clog << "CONV:  Solving: ";
            std::clog << m1.to_string() << Symbols::multiply << baseunits1.to_string();
            std::clog << " -> ";
            std::clog << m2.to_string() << Symbols::multiply << baseunits2.to_string();
            std::clog << '\n';
        }
        Result mag;
        if constexpr (Config::use_units_logarithmic) {
            if (utype == Utype::LOG && m2 == (Result)1) {
                mag = _convert_logarithmic(m1);
                goto converted;
            }
        }
        if constexpr (Config::use_units_temperature) {
            if (utype == Utype::TMP && m2 == (Result)1) {
                mag = _convert_temperature(m1);
                goto converted;
            }
        }
        if (utype == Utype::LIN)
            mag = _convert_linear(m1, m2);
        else
            throw puq::UnitException(
                "Conversion unavailable",
                "The conversion from base units `" + baseunits1.to_string() + "` to base units `" +
                    baseunits2.to_string() + "` is not available.",
                "Use compatible base units with a supported conversion.",
                __FILE__,
                __LINE__
            );
    converted:
        if constexpr (Config::debug_converter) {
            std::clog << "CONV:  Result: ";
            std::clog << mag.to_string() << Symbols::multiply << baseunits2.to_string();
            std::clog << '\n';
        }
        return mag;
    };

} // namespace snt::puq
