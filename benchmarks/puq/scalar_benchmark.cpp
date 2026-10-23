#include <benchmark/benchmark.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>
#include <snt/val/values.h>

#include <memory>
#include <vector>

using namespace snt;

// Construction isolates the current heap-backed representation of one scalar.
static void BM_Result_ScalarConstruct(benchmark::State& state) {
    for (auto _ : state) {
        puq::Result result(1.25);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Result_ScalarConstruct);

// Keep operands outside the timed loop so parsing and setup do not hide arithmetic costs.
static void BM_Result_ScalarAdd(benchmark::State& state) {
    const puq::Result left(1.25);
    const puq::Result right(2.5);
    for (auto _ : state) {
        auto result = left + right;
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Result_ScalarAdd);

static void BM_Result_ScalarMultiply(benchmark::State& state) {
    const puq::Result left(1.25);
    const puq::Result right(2.5);
    for (auto _ : state) {
        auto result = left * right;
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Result_ScalarMultiply);

static void BM_Result_ScalarAddWithUncertainty(benchmark::State& state) {
    const puq::Result left(1.25, 0.1);
    const puq::Result right(2.5, 0.2);
    for (auto _ : state) {
        auto result = left + right;
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Result_ScalarAddWithUncertainty);

// This control case shows whether a scalar optimization changes the array path.
static void BM_Result_ArrayAdd16(benchmark::State& state) {
    const puq::Result left(std::make_unique<val::ArrayValueFloat64>(std::vector<double>(16, 1.25)));
    const puq::Result right(std::make_unique<val::ArrayValueFloat64>(std::vector<double>(16, 2.5)));
    for (auto _ : state) {
        auto result = left + right;
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Result_ArrayAdd16);

// Quantity cases include unit handling but exclude repeated unit-expression parsing.
static void BM_Quantity_ScalarAddSameUnits(benchmark::State& state) {
    const puq::Quantity left(1.25, "m");
    const puq::Quantity right(2.5, "m");
    for (auto _ : state) {
        auto result = left + right;
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Quantity_ScalarAddSameUnits);

static void BM_Quantity_ScalarMultiplySameUnits(benchmark::State& state) {
    const puq::Quantity left(1.25, "m");
    const puq::Quantity right(2.5, "m");
    for (auto _ : state) {
        auto result = left * right;
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Quantity_ScalarMultiplySameUnits);

static void BM_Quantity_ScalarConvertKnownUnits(benchmark::State& state) {
    const puq::Quantity source(1.25, "m");
    const puq::Quantity target(1.0, "cm");
    for (auto _ : state) {
        auto result = source.convert(target);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Quantity_ScalarConvertKnownUnits);

BENCHMARK_MAIN();
