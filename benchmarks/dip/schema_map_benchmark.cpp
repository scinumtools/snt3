#include <algorithm>
#include <benchmark/benchmark.h>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <snt/dip/dip.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/val/values_number.h>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {

    using namespace snt;

    enum class Workload { Flat, ExplicitMap, SchemaMap, SchemaMapCondition, SchemaMapDefault, SchemaMapDefaultOverride };

    std::string number_text(const std::size_t index) {
        char buffer[64];
        const double value = static_cast<double>(index % 1000 + 1) / 10.0;
        const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
        if (result.ec != std::errc{})
            throw std::runtime_error("Could not format benchmark float value");
        std::string text(buffer, result.ptr);
        if (text.find_first_of(".eE") == std::string::npos)
            text += ".0";
        return text;
    }

    std::string item_name(const std::size_t index) {
        std::string name = "p";
        const auto digits = std::to_string(index);
        name.append(5 - std::min<std::size_t>(5, digits.size()), '0');
        name += digits;
        return name;
    }

    void append_schema_declarations(std::string& source, const bool with_condition, const bool with_default = false) {
        source += "$schema parameter_schema\n";
        source += with_default ? "  value float = 0.1\n" : "  value float\n";
        if (with_condition)
            source += "    !condition ({.} > 0)\n";
        source += "$schema config_schema\n";
        source += "  parameters map : parameter_schema\n";
    }

    std::string make_document(const std::size_t count, const Workload workload) {
        std::string source;
        source.reserve(count * 64 + 256);
        const bool schema_map = workload == Workload::SchemaMap || workload == Workload::SchemaMapCondition ||
                                workload == Workload::SchemaMapDefault || workload == Workload::SchemaMapDefaultOverride;
        if (schema_map)
            append_schema_declarations(
                source, workload == Workload::SchemaMapCondition,
                workload == Workload::SchemaMapDefault || workload == Workload::SchemaMapDefaultOverride
            );

        if (workload == Workload::Flat) {
            source += "config\n";
            source += "  parameters\n";
            for (std::size_t index = 0; index < count; ++index) {
                source += "    ";
                source += item_name(index);
                source += " float = ";
                source += number_text(index);
                source += '\n';
            }
            return source;
        }

        if (schema_map) {
            source += "config : config_schema\n";
            for (std::size_t index = 0; index < count; ++index) {
                source += "  parameters[";
                source += item_name(index);
                source += "]\n";
                if (workload != Workload::SchemaMapDefault) {
                    source += "    value = ";
                    source += number_text(index);
                    source += '\n';
                }
            }
            return source;
        }

        source += "config\n";
        source += "  parameters map\n";
        for (std::size_t index = 0; index < count; ++index) {
            source += "  parameters[";
            source += item_name(index);
            source += "]\n    value float = ";
            source += number_text(index);
            source += '\n';
        }
        return source;
    }

    std::string make_schema_only_document() {
        std::string source;
        append_schema_declarations(source, false);
        return source;
    }

    dip::Environment parse_document(const std::string& source) {
        dip::DIP parser;
        parser.add_string(source);
        return parser.parse();
    }

    void verify_values(const dip::Environment& environment, const std::size_t count, const bool flat, const bool default_values) {
        if (environment.nodes.size() != count)
            throw std::runtime_error("Unexpected number of evaluated DIPL value nodes");

        std::size_t seen = 0;
        const std::string prefix = flat ? "config.parameters." : "config.parameters[";
        for (const auto& node : environment.nodes.get_nodes()) {
            if (!node || node->path.name.rfind(prefix, 0) != 0)
                continue;
            if (!flat && node->path.name.find("].value") == std::string::npos)
                throw std::runtime_error("A map item value has an unexpected path");
            const auto* value = dynamic_cast<const val::ArrayValue<double>*>(node->value.get());
            if (!value || value->get_size() != 1)
                throw std::runtime_error("A benchmark parameter was not evaluated as a scalar float");
            if (default_values && std::abs(value->get_value(0) - 0.1) > 1e-12)
                throw std::runtime_error("A schema default did not reach its map item");
            ++seen;
        }
        if (seen != count)
            throw std::runtime_error("The parsed environment is missing benchmark parameters");
    }

    void BM_DipSchemaDeclarationsOnly(benchmark::State& state) {
        const std::string source = make_schema_only_document();
        state.counters["payload_bytes"] = static_cast<double>(source.size());
        const auto prepared = parse_document(source);
        if (prepared.nodes.size() != 0 || prepared.schemas.entries().size() != 2)
            throw std::runtime_error("Schema-only benchmark did not load its two schemas");
        for (auto _ : state) {
            state.PauseTiming();
            dip::DIP parser;
            parser.add_string(source);
            state.ResumeTiming();
            const auto environment = parser.parse();
            benchmark::DoNotOptimize(environment.schemas.entries().size());
        }
    }

    void BM_DipTypedWorkload(benchmark::State& state, const Workload workload) {
        const auto count = static_cast<std::size_t>(state.range(0));
        const auto source = make_document(count, workload);
        state.counters["payload_bytes"] = static_cast<double>(source.size());
        const bool flat = workload == Workload::Flat;
        const auto prepared = parse_document(source);
        verify_values(prepared, count, flat, workload == Workload::SchemaMapDefault);

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

    void BM_DipTypedFlat(benchmark::State& state) {
        BM_DipTypedWorkload(state, Workload::Flat);
    }

    void BM_DipTypedExplicitMap(benchmark::State& state) {
        BM_DipTypedWorkload(state, Workload::ExplicitMap);
    }

    void BM_DipTypedSchemaMap(benchmark::State& state) {
        BM_DipTypedWorkload(state, Workload::SchemaMap);
    }

    void BM_DipTypedSchemaMapCondition(benchmark::State& state) {
        BM_DipTypedWorkload(state, Workload::SchemaMapCondition);
    }

    void BM_DipTypedSchemaMapDefault(benchmark::State& state) {
        BM_DipTypedWorkload(state, Workload::SchemaMapDefault);
    }

    void BM_DipTypedSchemaMapDefaultOverride(benchmark::State& state) {
        BM_DipTypedWorkload(state, Workload::SchemaMapDefaultOverride);
    }

} // namespace

BENCHMARK(BM_DipSchemaDeclarationsOnly);
BENCHMARK(BM_DipTypedFlat)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DipTypedExplicitMap)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DipTypedSchemaMap)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DipTypedSchemaMapCondition)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DipTypedSchemaMapDefault)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DipTypedSchemaMapDefaultOverride)->ArgName("parameters")->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();
