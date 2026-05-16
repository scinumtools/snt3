#include <benchmark/benchmark.h>
#include <snt/puq/quantity.h>
#include <snt/puq/systems/unit_system.h>

using namespace snt;

static void BM_ParseEnergyPUQ(benchmark::State& state) {
  for (auto _ : state) {
    puq::Quantity q("123e4*kg*m2/s2");
    benchmark::DoNotOptimize(q);
  }
}
BENCHMARK(BM_ParseEnergyPUQ);

static void BM_UnitConversionEnergyPUQ(benchmark::State& state) {
  puq::Quantity q("123e4*kg*m2/s2");
  for (auto _ : state) {
    auto result = q.convert("J");
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_UnitConversionEnergyPUQ);

static void BM_SystemConversionEnergyPUQ(benchmark::State& state) {
  puq::Quantity q("123e4*kg*m2/s2");
  for (auto _ : state) {
    auto result = q.convert("ft*lbf", puq::SystemType::US);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_SystemConversionEnergyPUQ);

static void BM_ParseTemperaturePUQ(benchmark::State& state) {
  for (auto _ : state) {
    puq::Quantity q("45*Cel");
    benchmark::DoNotOptimize(q);
  }
}
BENCHMARK(BM_ParseTemperaturePUQ);

static void BM_UnitConversionTemperaturePUQ(benchmark::State& state) {
  puq::Quantity q("45*Cel");
  for (auto _ : state) {
    auto result = q.convert("degR");
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_UnitConversionTemperaturePUQ);

BENCHMARK_MAIN();
