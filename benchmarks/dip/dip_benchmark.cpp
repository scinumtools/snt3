#include <algorithm>
#include <benchmark/benchmark.h>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <map>
#include <mutex>
#include <snt/dip/dip.h>
#include <snt/dip/inspect/inspector.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/val/values_number.h>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {

    using namespace snt;

    std::string make_document(std::size_t count) {
        std::string source;
        source.reserve(count * 40 + 160);
        source += "experiment\n";
        source += "  name str = \"serialization benchmark\"\n";
        source += "  enabled bool = true\n";
        source += "  iterations int = 1000\n";
        source += "  timestep float = 0.00025\n";
        source += "  parameters\n";
        for (std::size_t i = 0; i < count; ++i) {
            const auto index = std::to_string(i);
            source += "    p";
            source.append(5 - std::min<std::size_t>(5, index.size()), '0');
            source += index;
            source += " float = ";
            char number[64];
            const double value = static_cast<double>(i % 1000) / 10.0;
            const auto formatted = std::to_chars(number, number + sizeof(number), value);
            if (formatted.ec != std::errc{})
                throw std::runtime_error("Could not format benchmark float value");
            const std::string formatted_value(number, formatted.ptr);
            source += formatted_value;
            if (formatted_value.find_first_of(".eE") == std::string::npos)
                source += ".0";
            if (i + 1 < count)
                source += '\n';
        }
        return source;
    }

    dip::Environment parse_document(const std::string& source) {
        dip::DIP parser;
        parser.add_string(source);
        return parser.parse();
    }

    void verify_environment(const dip::Environment& environment, std::size_t count) {
        if (environment.nodes.size() != count + 4)
            throw std::runtime_error("Unexpected number of evaluated DIPL value nodes");

        std::size_t parameter_count = 0;
        constexpr const char* prefix = "experiment.parameters.p";
        for (const auto& node : environment.nodes.get_nodes()) {
            if (!node || node->path.name.rfind(prefix, 0) != 0)
                continue;
            const auto* value = dynamic_cast<const val::ArrayValue<double>*>(node->value.get());
            if (!value || value->get_size() != 1)
                throw std::runtime_error("A benchmark parameter was not evaluated as a scalar float");

            const std::size_t index = std::stoul(node->path.name.substr(std::char_traits<char>::length(prefix)));
            const double expected = static_cast<double>(index % 1000) / 10.0;
            if (std::abs(value->get_value(0) - expected) > 1e-12)
                throw std::runtime_error("An evaluated parameter does not match its expected value");
            ++parameter_count;
        }
        if (parameter_count != count)
            throw std::runtime_error("The parsed environment is missing benchmark parameters");
    }

    const dip::Environment& prepared_environment(std::size_t count) {
        static std::map<std::size_t, dip::Environment> environments;
        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
        const auto existing = environments.find(count);
        if (existing != environments.end())
            return existing->second;

        auto environment = parse_document(make_document(count));
        verify_environment(environment, count);
        return environments.emplace(count, std::move(environment)).first->second;
    }

    double sum_float_values(const dip::ValueNode::ListType& nodes) {
        double sum = 0.0;
        for (const auto& node : nodes) {
            if (!node || !node->value)
                continue;
            if (const auto* value = dynamic_cast<const val::ArrayValue<double>*>(node->value.get()))
                sum += value->get_value(0);
        }
        return sum;
    }

    void BM_DipAddString(benchmark::State& state) {
        const auto count = static_cast<std::size_t>(state.range(0));
        const auto source = make_document(count);
        state.counters["payload_bytes"] = static_cast<double>(source.size());
        for (auto _ : state) {
            dip::DIP parser;
            parser.add_string(source);
            benchmark::DoNotOptimize(parser);
        }
        state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    }

    void BM_DipParseOnly(benchmark::State& state) {
        const auto count = static_cast<std::size_t>(state.range(0));
        const auto source = make_document(count);
        state.counters["payload_bytes"] = static_cast<double>(source.size());
        (void)prepared_environment(count);
        for (auto _ : state) {
            state.PauseTiming();
            dip::DIP parser;
            parser.add_string(source);
            state.ResumeTiming();
            auto environment = parser.parse();
            benchmark::DoNotOptimize(environment.nodes.size());
        }
        state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    }

    void BM_DipParseEndToEnd(benchmark::State& state) {
        const auto count = static_cast<std::size_t>(state.range(0));
        const auto source = make_document(count);
        state.counters["payload_bytes"] = static_cast<double>(source.size());
        (void)prepared_environment(count);
        for (auto _ : state) {
            auto environment = parse_document(source);
            benchmark::DoNotOptimize(environment.nodes.size());
        }
        state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    }

    void BM_DipSelectSnapshots(benchmark::State& state) {
        const auto count = static_cast<std::size_t>(state.range(0));
        const auto& environment = prepared_environment(count);
        dip::Inspector inspector(environment);
        for (auto _ : state) {
            auto snapshots = inspector.select("?");
            benchmark::DoNotOptimize(sum_float_values(snapshots));
        }
        state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    }

    void BM_DipReadOwnedNodes(benchmark::State& state) {
        const auto count = static_cast<std::size_t>(state.range(0));
        const auto& environment = prepared_environment(count);
        for (auto _ : state) {
            benchmark::DoNotOptimize(sum_float_values(environment.nodes.get_nodes()));
        }
        state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    }

} // namespace

BENCHMARK(BM_DipAddString)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DipParseOnly)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DipParseEndToEnd)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DipSelectSnapshots)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DipReadOwnedNodes)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();
