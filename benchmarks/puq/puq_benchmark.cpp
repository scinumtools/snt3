#include <benchmark/benchmark.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/solver/unit_solver.h>
#include <snt/puq/systems/unit_system.h>

using namespace snt;

static void BM_SNT_PUQ_UnitSolver_BaseUnit(benchmark::State& state) {
    puq::UnitSolver solver;
    puq::UnitSolver::cache.clear();
    for (auto _ : state) {
        puq::UnitAtom a = solver.eval("g");
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(BM_SNT_PUQ_UnitSolver_BaseUnit);

static void BM_SNT_PUQ_UnitSolver_ScalingFactor(benchmark::State& state) {
    puq::UnitSolver solver;
    puq::UnitSolver::cache.clear();
    for (auto _ : state) {
        puq::UnitAtom a = solver.eval("kg");
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(BM_SNT_PUQ_UnitSolver_ScalingFactor);

static void BM_SNT_PUQ_UnitSolver_Arithmetics(benchmark::State& state) {
    puq::UnitSolver solver;
    puq::UnitSolver::cache.clear();
    for (auto _ : state) {
        puq::UnitAtom a = solver.eval("1.234e5*kg*m2/s2");
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(BM_SNT_PUQ_UnitSolver_Arithmetics);

static void BM_SNT_PUQ_UnitSolver_Uncertainties(benchmark::State& state) {
    puq::UnitSolver solver;
    puq::UnitSolver::cache.clear();
    for (auto _ : state) {
        puq::UnitAtom a = solver.eval("1.234(56)e7*kg*m2/s2");
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(BM_SNT_PUQ_UnitSolver_Uncertainties);

static void BM_SNT_PUQ_Measurement_Uncertainties(benchmark::State& state) {
    puq::UnitSolver::cache.clear();
    for (auto _ : state) {
        puq::Measurement m("1.234(56)e7*kg*m2/s2");
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(BM_SNT_PUQ_Measurement_Uncertainties);

static void BM_SNT_PUQ_Quantity_Uncertainties(benchmark::State& state) {
    puq::UnitSolver::cache.clear();
    for (auto _ : state) {
        puq::Quantity q("1.234(56)e7*kg*m2/s2");
        benchmark::DoNotOptimize(q);
    }
}
BENCHMARK(BM_SNT_PUQ_Quantity_Uncertainties);

static void BM_SNT_PUQ_ParseEnergy(benchmark::State& state) {
    for (auto _ : state) {
        puq::Quantity q("123e4*kg*m2/s2");
        benchmark::DoNotOptimize(q);
    }
}
BENCHMARK(BM_SNT_PUQ_ParseEnergy);

static void BM_SNT_PUQ_UnitConversionEnergy(benchmark::State& state) {
    puq::Quantity q("123e4*kg*m2/s2");
    for (auto _ : state) {
        auto result = q.convert("J");
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SNT_PUQ_UnitConversionEnergy);

static void BM_SNT_PUQ_SystemUSConversionEnergy(benchmark::State& state) {
    puq::Quantity q("123e4*kg*m2/s2");
    for (auto _ : state) {
        auto result = q.convert("ft*lbf", puq::SystemType::US);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SNT_PUQ_SystemUSConversionEnergy);

static void BM_SNT_PUQ_SystemESUConversionEnergy(benchmark::State& state) {
    puq::Quantity q("123e4*kg*m2/s2");
    for (auto _ : state) {
        auto result = q.convert("erg", puq::SystemType::ESU);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SNT_PUQ_SystemESUConversionEnergy);

static void BM_SNT_PUQ_ParseTemperature(benchmark::State& state) {
    for (auto _ : state) {
        puq::Quantity q("45*Cel");
        benchmark::DoNotOptimize(q);
    }
}
BENCHMARK(BM_SNT_PUQ_ParseTemperature);

static void BM_SNT_PUQ_UnitConversionTemperature(benchmark::State& state) {
    puq::Quantity q("45*Cel");
    for (auto _ : state) {
        auto result = q.convert("degR");
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SNT_PUQ_UnitConversionTemperature);

BENCHMARK_MAIN();
