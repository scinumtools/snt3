#include <benchmark/benchmark.h>
#include <snt/puq/quantity.h>
#include <snt/val/values.h>

#include <memory>
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

// These cases isolate the C++ storage and conversion path from Python/NumPy.
static void BM_ArrayValue_Construct(benchmark::State& state) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    for (auto _ : state) {
        auto array = std::make_unique<val::ArrayValueFloat64>(values);
        benchmark::DoNotOptimize(array);
    }
}

static void BM_Quantity_Construct(benchmark::State& state) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    for (auto _ : state) {
        auto quantity = quantity_from(values, "m");
        benchmark::DoNotOptimize(quantity);
    }
}

static void BM_Quantity_Convert(benchmark::State& state, const char* source_units, const char* target_units) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    const auto source = quantity_from(values, source_units);
    const puq::Quantity target(1.0, target_units);
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
    for (auto _ : state) {
        auto converted = source.convert(target);
        benchmark::DoNotOptimize(converted);
    }
}

static void BM_Quantity_AddArrays(benchmark::State& state) {
    const auto values = values_for(static_cast<std::size_t>(state.range(0)));
    const auto left = quantity_from(values, "m");
    const auto right = quantity_from(std::vector<double>(values.size(), 2.0), "m");
    for (auto _ : state) {
        auto sum = left + right;
        benchmark::DoNotOptimize(sum);
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

BENCHMARK_MAIN();
