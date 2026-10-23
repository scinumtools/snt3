#include <benchmark/benchmark.h>
#include <snt/puq/quantity.h>
#include <snt/val/values.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace snt;

static std::vector<double> values_for(std::size_t size) {
    std::vector<double> values(size);
    for (std::size_t i = 0; i < size; ++i)
        values[i] = 30.0 * static_cast<double>(i) / static_cast<double>(size);
    return values;
}

static puq::Quantity quantity_from(const std::vector<double>& values, const char* units) {
    return puq::Quantity(std::make_unique<val::ArrayValueFloat64>(values), units);
}

static void check_values(const val::BaseValue* value, const std::vector<double>& expected, const char* label) {
    if (!value)
        throw std::runtime_error(std::string(label) + " is missing");
    const val::ArrayValueFloat64 actual(value);
    if (actual.get_shape() != val::Array::ShapeType{expected.size()})
        throw std::runtime_error(std::string(label) + " has an unexpected shape");
    const auto values = actual.get_values();
    if (values.size() != expected.size())
        throw std::runtime_error(std::string(label) + " has an unexpected size");
    for (std::size_t i = 0; i < values.size(); ++i) {
        const double tolerance = 1e-12 * std::max(1.0, std::abs(expected[i]));
        if (!std::isfinite(values[i]) || std::abs(values[i] - expected[i]) > tolerance)
            throw std::runtime_error(std::string(label) + " does not match its expected values");
    }
}

static void check_quantity(
    const puq::Quantity& quantity,
    const std::vector<double>& expected,
    const std::vector<double>* expected_uncertainty = nullptr
) {
    check_values(quantity.measurement.result.estimate.get(), expected, "Quantity estimate");
    if (expected_uncertainty)
        check_values(quantity.measurement.result.uncertainty.get(), *expected_uncertainty, "Quantity uncertainty");
    else if (quantity.measurement.result.uncertainty)
        throw std::runtime_error("Unexpected quantity uncertainty");
}

// These cases isolate the C++ storage and conversion path from Python/NumPy.
static void BM_ArrayValue_Construct(benchmark::State& state) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    val::ArrayValueFloat64 checked(values);
    check_values(&checked, values, "Array construction");
    for (auto _ : state) {
        auto array = std::make_unique<val::ArrayValueFloat64>(values);
        benchmark::DoNotOptimize(array);
    }
}

static void BM_Quantity_Construct(benchmark::State& state) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    const auto checked = quantity_from(values, "m");
    check_quantity(checked, values);
    for (auto _ : state) {
        auto quantity = quantity_from(values, "m");
        benchmark::DoNotOptimize(quantity);
    }
}

static void BM_Quantity_Convert(benchmark::State& state, const char* source_units, const char* target_units) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    const auto source = quantity_from(values, source_units);
    const puq::Quantity target(1.0, target_units);
    const auto checked = source.convert(target);
    std::vector<double> expected = values;
    if (std::string(source_units) == "m" && std::string(target_units) == "mm") {
        for (double& value : expected)
            value *= 1000.0;
    } else if (std::string(source_units) == "Cel" && std::string(target_units) == "K") {
        for (double& value : expected)
            value += 273.15;
    } else if (std::string(source_units) == "dBm" && std::string(target_units) == "mW") {
        for (double& value : expected)
            value = std::pow(10.0, value / 10.0);
    }
    check_quantity(checked, expected);
    for (auto _ : state) {
        auto converted = source.convert(target);
        benchmark::DoNotOptimize(converted);
    }
}

static void BM_Quantity_ConvertWithUncertainty(benchmark::State& state) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    const std::vector<double> errors(values.size(), 0.25);
    const puq::Quantity source(
        std::make_unique<val::ArrayValueFloat64>(values),
        std::make_unique<val::ArrayValueFloat64>(errors),
        "m"
    );
    const puq::Quantity target(1.0, "mm");
    const auto checked = source.convert(target);
    std::vector<double> expected_values = values;
    std::vector<double> expected_errors = errors;
    for (double& value : expected_values)
        value *= 1000.0;
    for (double& value : expected_errors)
        value *= 1000.0;
    check_quantity(checked, expected_values, &expected_errors);
    for (auto _ : state) {
        auto converted = source.convert(target);
        benchmark::DoNotOptimize(converted);
    }
}

static void BM_Quantity_AddArrays(benchmark::State& state) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    const auto left = quantity_from(values, "m");
    const auto right = quantity_from(std::vector<double>(values.size(), 2.0), "m");
    const auto checked = left + right;
    std::vector<double> expected = values;
    for (double& value : expected)
        value += 2.0;
    check_quantity(checked, expected);
    for (auto _ : state) {
        auto sum = left + right;
        benchmark::DoNotOptimize(sum);
    }
}

// Split multiplication into its array, Result, Measurement, and Quantity layers.
// The m*s versus m*m pair checks whether unit combination explains the paper
// benchmark's multiplication/square timing difference.
static void BM_MultiplyLayers(benchmark::State& state, const char* right_units, int layer, bool same_operand) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    const auto left = quantity_from(values, "m");
    const auto right = quantity_from(values, right_units);
    const auto& rhs = same_operand ? left : right;
    const auto expected = [&] {
        std::vector<double> result(values.size());
        for (std::size_t i = 0; i < values.size(); ++i)
            result[i] = values[i] * values[i];
        return result;
    }();
    const auto checked = left * rhs;
    check_quantity(checked, expected);

    for (auto _ : state) {
        if (layer == 2) {
            auto product = left * rhs;
            benchmark::DoNotOptimize(product);
        } else if (layer == 1) {
            auto product = left.measurement.result * rhs.measurement.result;
            benchmark::DoNotOptimize(product);
        } else {
            auto product = left.measurement.result.estimate->math_mul(rhs.measurement.result.estimate.get());
            benchmark::DoNotOptimize(product);
        }
    }
}

// Compare the normal vector result allocation with an uninitialized result
// buffer, which avoids zero-filling values that the arithmetic loop overwrites.
static void BM_ArrayMultiply_UninitializedOutput(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto left = values_for(size);
    const auto right = values_for(size);
    auto check = std::make_unique_for_overwrite<double[]>(size);
    for (std::size_t i = 0; i < size; ++i)
        check[i] = left[i] * right[i];
    if (check[0] != left[0] * right[0] || check[size - 1] != left[size - 1] * right[size - 1])
        throw std::runtime_error("Uninitialized output probe produced incorrect values");

    for (auto _ : state) {
        auto output = std::make_unique_for_overwrite<double[]>(size);
        for (std::size_t i = 0; i < size; ++i)
            output[i] = left[i] * right[i];
        benchmark::DoNotOptimize(output.get());
        benchmark::ClobberMemory();
    }
}

static void BM_ArrayMultiply_VectorOutput(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto left = values_for(size);
    const auto right = values_for(size);
    for (auto _ : state) {
        std::vector<double> output(size);
        for (std::size_t i = 0; i < size; ++i)
            output[i] = left[i] * right[i];
        benchmark::DoNotOptimize(output.data());
        benchmark::ClobberMemory();
    }
}

static void BM_Quantity_MultiplyWithUncertainty(benchmark::State& state) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    const std::vector<double> errors(values.size(), 0.25);
    const puq::Quantity left(
        std::make_unique<val::ArrayValueFloat64>(values),
        std::make_unique<val::ArrayValueFloat64>(errors),
        "m"
    );
    const puq::Quantity right(
        std::make_unique<val::ArrayValueFloat64>(std::vector<double>(values.size(), 2.0)),
        std::make_unique<val::ArrayValueFloat64>(errors),
        "s"
    );
    const auto checked = left * right;
    std::vector<double> expected_values(values.size());
    std::vector<double> expected_errors(values.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        expected_values[i] = values[i] * 2.0;
        expected_errors[i] = std::max(
            std::abs((values[i] + 0.25) * 2.25 - expected_values[i]),
            std::abs((values[i] - 0.25) * 1.75 - expected_values[i])
        );
    }
    check_quantity(checked, expected_values, &expected_errors);
    for (auto _ : state) {
        auto product = left * right;
        benchmark::DoNotOptimize(product);
    }
}

#define ARRAY_LENGTHS ->Arg(1000)->Arg(10000)->Arg(100000)
BENCHMARK(BM_ArrayValue_Construct) ARRAY_LENGTHS;
BENCHMARK(BM_Quantity_Construct) ARRAY_LENGTHS;
BENCHMARK_CAPTURE(BM_Quantity_Convert, m_to_m, "m", "m") ARRAY_LENGTHS;
BENCHMARK_CAPTURE(BM_Quantity_Convert, m_to_mm, "m", "mm") ARRAY_LENGTHS;
BENCHMARK_CAPTURE(BM_Quantity_Convert, Cel_to_K, "Cel", "K") ARRAY_LENGTHS;
BENCHMARK_CAPTURE(BM_Quantity_Convert, dBm_to_mW, "dBm", "mW") ARRAY_LENGTHS;
BENCHMARK(BM_Quantity_ConvertWithUncertainty) ARRAY_LENGTHS;
BENCHMARK(BM_Quantity_AddArrays) ARRAY_LENGTHS;
BENCHMARK_CAPTURE(BM_MultiplyLayers, raw_array_multiply_separate, "s", 0, false)->Arg(1000000);
BENCHMARK_CAPTURE(BM_MultiplyLayers, raw_array_multiply_same, "m", 0, true)->Arg(1000000);
BENCHMARK_CAPTURE(BM_MultiplyLayers, result_multiply_separate, "s", 1, false)->Arg(1000000);
BENCHMARK_CAPTURE(BM_MultiplyLayers, quantity_multiply_ms, "s", 2, false)->Arg(1000000);
BENCHMARK_CAPTURE(BM_MultiplyLayers, quantity_square_same_operand, "m", 2, true)->Arg(1000000);
BENCHMARK(BM_ArrayMultiply_UninitializedOutput)->Arg(1000000);
BENCHMARK(BM_ArrayMultiply_VectorOutput)->Arg(1000000);
BENCHMARK(BM_Quantity_MultiplyWithUncertainty) ARRAY_LENGTHS;

BENCHMARK_MAIN();
