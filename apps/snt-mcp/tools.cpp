#include "../comparison_json.h"
#include "../service_support.h"
#include "mcp.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <snt/api/dip_compare.h>
#include <snt/api/dip_parse.h>
#include <snt/api/dip_semantic.h>
#include <snt/api/puq_convert.h>
#include <snt/api/puq_eval.h>
#include <snt/api/puq_info.h>
#include <snt/api/puq_list.h>
#include <snt/dip/artifact.h>
#ifdef SNT_ENABLE_REPORT
#include <snt/api/dip_report.h>
#endif

namespace snt::mcp {
    namespace {
        namespace fs = std::filesystem;

        Json text(const std::string& value = "") {
            return {{"type", "string"}, {"default", value}, {"maxLength", 262144}};
        }
        Json count(std::size_t value) {
            return {{"type", "integer"}, {"minimum", 0}, {"maximum", 1000}, {"default", value}};
        }
        Json choice(Json values, const std::string& value) {
            return {{"type", "string"}, {"enum", std::move(values)}, {"default", value}};
        }
        Json strings() {
            return {{"type", "array"}, {"items", text()}, {"maxItems", 1000}, {"default", Json::array()}};
        }
        Json environment() {
            return {{"environment", text("default")}};
        }
        Json selection(std::size_t elements = 0) {
            return {{"query", text("?")}, {"limit", count(100)}, {"max_value_elements", count(elements)}};
        }
        Json record_graph() {
            return {{"type", "boolean"}, {"default", false}};
        }

        // Validate only the JSON Schema vocabulary used in this tool catalogue.
        // The same definitions drive discovery, defaults and argument checks.
        void validate(const Json& value, const Json& schema, const std::string& key) {
            const auto type = schema.at("type").get<std::string>();
            const bool valid = type == "string"    ? value.is_string()
                               : type == "integer" ? value.is_number_integer()
                               : type == "boolean" ? value.is_boolean()
                               : type == "array"   ? value.is_array()
                                                   : false;
            if (!valid)
                throw std::invalid_argument(key + " must be " + type + ".");
            if (schema.contains("enum") &&
                std::find(schema["enum"].begin(), schema["enum"].end(), value) == schema["enum"].end())
                throw std::invalid_argument("Invalid " + key + "; expected one of " + schema["enum"].dump());
            if (type == "integer" && (value < schema.at("minimum") || value > schema.at("maximum")))
                throw std::invalid_argument(
                    key + " must be between " + schema.at("minimum").dump() + " and " + schema.at("maximum").dump() +
                    "."
                );
            if (type == "string") {
                // JSON Schema maxLength counts Unicode code points, not UTF-8 bytes.
                const auto& s = value.get_ref<const std::string&>();
                const auto length =
                    std::count_if(s.begin(), s.end(), [](unsigned char c) { return (c & 0xc0) != 0x80; });
                if (schema.contains("maxLength") && length > schema["maxLength"].get<std::ptrdiff_t>())
                    throw std::invalid_argument(key + " is too long.");
            }
            if (type == "array") {
                if (value.size() > schema.at("maxItems").get<std::size_t>())
                    throw std::invalid_argument(key + " has too many items.");
                for (const auto& item : value)
                    validate(item, schema.at("items"), key + "[]");
            }
        }

        void configure_input(api::DIPParse& parser, const fs::path& path, const Json& overrides) {
            switch (dip::detect_artifact(path)) {
            case dip::ArtifactKind::DIPH5:
                if (!overrides.empty())
                    throw std::invalid_argument("Overrides require DIPL source input, not a DIPH5 snapshot.");
                parser.argument_load(path.string());
                break;
            case dip::ArtifactKind::Project:
                parser.argument_add("project", {path.string()});
                break;
            default:
                parser.argument_add("file", {path.string()});
                break;
            }
            for (const auto& body : overrides)
                parser.argument_add("override_string", {body.get<std::string>()});
        }

        std::string mime_type(const std::string& format) {
            if (format == "json" || format == "json-metadata")
                return "application/json";
            if (format == "yaml")
                return "application/yaml";
            if (format == "html")
                return "text/html";
            if (format == "md")
                return "text/markdown";
            return "text/plain";
        }

        std::string read_text(const fs::path& path) {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
                throw std::runtime_error("Cannot read generated artifact.");
            std::string value(message_limit + 1, '\0');
            stream.read(value.data(), static_cast<std::streamsize>(value.size()));
            value.resize(static_cast<std::size_t>(stream.gcount()));
            if (value.size() > message_limit)
                throw std::runtime_error("Generated artifact exceeds 4 MiB.");
            return value;
        }

        template <class Command> void configure_quantity(Command& command, const Json& args) {
            if (!args["input_system"].get<std::string>().empty())
                command.argument_input_system(args["input_system"]);
            if (!args["output_system"].get<std::string>().empty())
                command.argument_output_system(args["output_system"]);
            if (!args["output_quantity"].get<std::string>().empty())
                command.argument_output_quantity(args["output_quantity"]);
        }
    } // namespace

    void Tools::add(
        std::string name,
        std::string description,
        Json properties,
        Json required,
        std::function<Json(const Json&)> invoke
    ) {
        describe_arguments(properties);
        Json definition = {
            {"name", name},
            {"description", std::move(description)},
            {"inputSchema",
             {{"type", "object"},
              {"properties", std::move(properties)},
              {"required", std::move(required)},
              {"additionalProperties", false}}},
            {"outputSchema", output_schema(name)},
            {"annotations", {{"readOnlyHint", true}, {"destructiveHint", false}, {"openWorldHint", false}}}
        };
        tools_.emplace(std::move(name), Tool{std::move(definition), std::move(invoke)});
    }

    Tools::Tools(Projects projects, Storage* storage) : projects_(std::move(projects)), storage_(storage) {
        const auto none = Json::array();
        add("dipl_environments",
            "List named DIPL inputs and DIPH5 snapshots configured at startup.",
            Json::object(),
            none,
            [this](const Json&) { return environments(); });

        auto listing = environment();
        listing.update(selection());
        listing["cursor"] = text();
        listing.update({{"tags_all", strings()}, {"tags_any", strings()}, {"tags_none", strings()}});
        add("dipl_list",
            "Discover nodes by path and explicit tags. ? selects all, ?group. a subtree. "
            "Values are omitted by default. Narrow the query when truncated.",
            listing,
            none,
            [this](const Json& args) {
                if (args["cursor"] != "")
                    return storage_->page({}, args["limit"], args["cursor"]);
                dip::TagFilter tags{args["tags_all"], args["tags_any"], args["tags_none"]};
                auto values = Json::parse(
                    api::DIPSemantic(project(args))
                        .list_json(
                            args["query"], tags, std::numeric_limits<std::size_t>::max(), args["max_value_elements"]
                        )
                );
                return storage_->page(std::move(values), args["limit"], "");
            });
        auto description = environment();
        description.update(
            {{"path", text()}, {"max_value_elements", count(16)}, {"record_dependencies", record_graph()}}
        );
        add("dipl_describe",
            "Inspect value, units, constraints, metadata, provenance and optional dependencies. "
            "Snapshots expose only their saved dependency graph.",
            description,
            {"path"},
            [this](const Json& args) {
                return Json::parse(
                    api::DIPSemantic(project(args), args["record_dependencies"])
                        .describe_json(args["path"], args["max_value_elements"])
                );
            });
        add("dipl_schemas",
            "Inspect schema definitions, applications and availability flags.",
            environment(),
            none,
            [this](const Json& args) { return Json::parse(api::DIPSemantic(project(args)).schemas_json()); });
        add("dipl_sources",
            "Inspect source identities, metadata and fingerprints.",
            environment(),
            none,
            [this](const Json& args) { return Json::parse(api::DIPSemantic(project(args)).sources_json()); });
        auto contract = environment();
        contract["path"] = text();
        add("dipl_override_contract",
            "Inspect allowed override targets, types, units and enforced constraints.",
            contract,
            {"path"},
            [this](const Json& args) {
                return Json::parse(api::DIPSemantic(project(args)).override_contract_json(args["path"]));
            });
        auto preview = environment();
        preview.update({{"overrides", strings()}, {"max_details", count(50)}, {"record_dependencies", record_graph()}});
        add("dipl_preview",
            "Evaluate unwrapped DIPL override bodies without saving changes. Check candidate_valid "
            "and diagnostics; invalid candidates are normal results. Requires source input, not a snapshot. "
            "An empty overrides array validates the baseline.",
            preview,
            {"overrides"},
            [this](const Json& args) {
                std::vector<dip::PreviewOverride> overrides;
                for (const auto& value : args["overrides"])
                    overrides.push_back({dip::PreviewOverride::Kind::Text, value.get<std::string>()});
                return Json::parse(
                    api::DIPSemantic(project(args), args["record_dependencies"])
                        .preview_json(overrides, args["max_details"])
                );
            });
        auto evaluate = selection(16);
        evaluate["source"] = text();
        add("dipl_evaluate",
            "Parse, validate and evaluate inline DIPL. Use named projects for multiple files. "
            "Evaluation runs with the server's filesystem permissions.",
            evaluate,
            {"source"},
            [this](const Json& args) {
                app::TemporaryDirectory scratch(storage_->workspace());
                const auto path = scratch.path / "input.dip";
                {
                    std::ofstream stream(path, std::ios::binary);
                    stream << args["source"].get<std::string>();
                    if (!stream)
                        throw std::runtime_error("Cannot write inline DIPL.");
                }
                auto values = Json::parse(
                    api::DIPSemantic(path).list_json(
                        args["query"], {}, std::numeric_limits<std::size_t>::max(), args["max_value_elements"]
                    )
                );
                return storage_->page(std::move(values), args["limit"], "");
            });
        add("dipl_compare",
            "Compare two named DIPH5 snapshots. Return counts and structured differences with paths, fields, "
            "before/after summaries and bounded array indices. Differences are successful results. "
            "Full scope also compares persisted metadata and provenance.",
            {{"before", text()},
             {"after", text()},
             {"scope", choice({"effective", "full"}, "effective")},
             {"max_details", count(50)},
             {"max_array_examples", count(3)}},
            {"before", "after"},
            [this](const Json& args) {
                const auto& before = project(args, "before");
                const auto& after = project(args, "after");
                if (dip::detect_artifact(before) != dip::ArtifactKind::DIPH5 ||
                    dip::detect_artifact(after) != dip::ArtifactKind::DIPH5)
                    throw std::invalid_argument("Comparison requires two DIPH5 snapshots.");
                api::DIPCompare compare(before, after);
                dip::ComparisonOptions options;
                if (args["scope"] == "full")
                    options.scope = dip::ComparisonScope::Full;
                options.max_array_examples = args["max_array_examples"];
                compare.set_options(options);
                compare.set_max_details(args["max_details"]);
                const auto result = compare.compare();
                auto value = app::comparison_json(result, args["max_details"]);
                value["result"] = compare.execute();
                return value;
            });
        auto export_options = environment();
        export_options["overrides"] = strings();
        export_options["format"] =
            choice({"cpp", "c", "fortran", "rust", "julia", "json", "json-metadata", "yaml"}, "json");
        add("dipl_export",
            "Generate static parameters and return text without writing into the project. Optional overrides "
            "are unwrapped DIPL bodies, as in dipl_preview; they require source input, not a snapshot. "
            "Each call re-evaluates current inputs and validates the overrides.",
            export_options,
            none,
            [this](const Json& args) {
                app::TemporaryDirectory scratch(storage_->workspace());
                const auto path = scratch.path / "parameters.txt";
                api::DIPParse parser;
                configure_input(parser, project(args), args["overrides"]);
                parser.argument_generate(args["format"], path.string());
                parser.execute();
                Json result = {
                    {"format", args["format"]},
                    {"artifact",
                     storage_->put_file(
                         path, "export", mime_type(args["format"]), "parameters." + args["format"].get<std::string>()
                     )},
                    {"inline", fs::file_size(path) <= chunk_limit}
                };
                if (result["inline"] == true)
                    result["content"] = read_text(path);
                return result;
            });
#ifdef SNT_ENABLE_REPORT
        auto report_options = environment();
        report_options["overrides"] = strings();
        report_options["format"] = choice({"tex", "md", "rst", "html", "typ", "txt", "json"}, "md");
        add("dipl_report",
            "Generate a textual parameter report without writing into the project. Optional overrides "
            "are unwrapped DIPL bodies, as in dipl_preview; they require source input, not a snapshot. "
            "Each call re-evaluates current inputs and validates the overrides.",
            report_options,
            none,
            [this](const Json& args) {
                using Format = dip::report::ReportFormat;
                const std::map<std::string, Format> formats{
                    {"tex", Format::Tex},
                    {"md", Format::Markdown},
                    {"rst", Format::Rst},
                    {"html", Format::Html},
                    {"typ", Format::Typst},
                    {"txt", Format::Text},
                    {"json", Format::Json}
                };
                app::TemporaryDirectory scratch(storage_->workspace());
                const auto path = scratch.path / "report.txt";
                api::DIPParse parser;
                configure_input(parser, project(args), args["overrides"]);
                auto env = parser.evaluate(true);
                api::generate_dip_report(env, formats.at(args["format"]), path);
                Json result = {
                    {"format", args["format"]},
                    {"artifact",
                     storage_->put_file(
                         path, "report", mime_type(args["format"]), "report." + args["format"].get<std::string>()
                     )},
                    {"inline", fs::file_size(path) <= chunk_limit}
                };
                if (result["inline"] == true)
                    result["content"] = read_text(path);
                return result;
            });
#endif
        auto snapshot = environment();
        snapshot["overrides"] = strings();
        add("dipl_snapshot",
            "Validate inputs with optional overrides and create an immutable evaluated DIPH5 candidate. "
            "Use the returned environment identifier for inspection, comparison, export and reports. "
            "Includes dependency graph and source fingerprints. Expires after one hour or server shutdown; download to "
            "retain.",
            snapshot,
            none,
            [this](const Json& args) {
                app::TemporaryDirectory scratch(storage_->workspace());
                auto path = scratch.path / "candidate.diph5";
                api::DIPParse parser;
                configure_input(parser, project(args), args["overrides"]);
                auto environment = parser.evaluate(true);
                environment.save(path);
                auto artifact = storage_->put_file(path, "snapshot", "application/x-hdf5", "candidate.diph5");
                return Json{{"environment", artifact["id"]}, {"artifact", artifact}, {"overrides", args["overrides"]}};
            });
        add("artifact_read",
            "Read generated output in bounded base64 chunks. Append decoded bytes until next_offset is null. "
            "Verify the complete file using artifact.sha256. Expired identifiers require regeneration.",
            {{"id", text()},
             {"offset", {{"type", "integer"}, {"minimum", 0}, {"maximum", artifact_budget}, {"default", 0}}},
             {"length", {{"type", "integer"}, {"minimum", 1}, {"maximum", chunk_limit}, {"default", chunk_limit}}}},
            {"id"},
            [this](const Json& args) { return storage_->read(args["id"], args["offset"], args["length"]); });
        add("puq",
            "Evaluate/convert PUEL quantities, inspect them or list definitions. convert requires output_units. "
            "Results preserve units and uncertainties as formatted text.",
            {{"operation", choice({"eval", "convert", "info", "list"}, "eval")},
             {"expression", text()},
             {"output_units", text()},
             {"input_system", text()},
             {"output_system", text()},
             {"output_quantity", text()},
             {"category", choice({"prefix", "base", "deriv", "log", "temp", "const", "quant", "sys"}, "sys")}},
            {"operation"},
            [](const Json& args) {
                const std::string operation = args["operation"], expression = args["expression"];
                const std::string input = args["input_system"], units = args["output_units"];
                if (operation != "list" && expression.empty())
                    throw std::invalid_argument("expression is required.");
                if ((operation == "info" || operation == "list") &&
                    (!units.empty() || args["output_system"] != "" || args["output_quantity"] != ""))
                    throw std::invalid_argument("Output conversion options require eval or convert.");
                std::string result;
                if (operation == "eval") {
                    api::PUQEval command(expression);
                    configure_quantity(command, args);
                    if (!units.empty())
                        command.argument_output_units(units);
                    result = command.execute();
                } else if (operation == "convert") {
                    if (units.empty())
                        throw std::invalid_argument("convert requires output_units.");
                    api::PUQConvert command(expression, units);
                    configure_quantity(command, args);
                    result = command.execute();
                } else if (operation == "info") {
                    api::PUQInfo command(expression);
                    if (!input.empty())
                        command.argument_input_system(input);
                    result = command.execute();
                } else {
                    api::PUQList command(args["category"]);
                    if (!input.empty())
                        command.argument_system(input);
                    result = command.execute();
                }
                return Json{{"result", result}};
            });
    }

    std::filesystem::path Tools::project(const Json& args, const std::string& key) const {
        const auto name = args.at(key).get<std::string>();
        const auto found = projects_.find(name);
        if (found == projects_.end() && name.rfind("artifact-", 0) == 0 && storage_) {
            if (storage_->metadata(name)["kind"] != "snapshot")
                throw ServiceError("argument.invalid", "Only snapshot artifacts can be used as environments.");
            return storage_->path(name);
        }
        if (found == projects_.end())
            throw std::invalid_argument("Unknown environment '" + name + "'; call dipl_environments first.");
        return found->second;
    }
    Json Tools::environments() const {
        Json values = Json::array();
        for (const auto& [name, path] : projects_) {
            const bool snapshot = dip::detect_artifact(path) == dip::ArtifactKind::DIPH5;
            bool report = false;
#ifdef SNT_ENABLE_REPORT
            report = true;
#endif
            values.push_back(
                {{"name", name},
                 {"input", path.filename().string()},
                 {"snapshot", snapshot},
                 {"capabilities", {{"overrides", !snapshot}, {"snapshot", true}, {"export", true}, {"report", report}}}}
            );
        }
        return {{"environments", values}};
    }
    Json Tools::list() const {
        Json result = Json::array();
        for (const auto& [name, tool] : tools_)
            result.push_back(tool.definition);
        return result;
    }
    bool Tools::contains(const std::string& name) const {
        return tools_.count(name) != 0;
    }
    Json Tools::call(const std::string& name, const Json& arguments) const {
        const auto& tool = tools_.at(name);
        const auto& schema = tool.definition["inputSchema"];
        if (!arguments.is_object())
            throw std::invalid_argument("Tool arguments must be an object.");
        Json args = arguments;
        for (const auto& required : schema["required"])
            if (!args.contains(required.get<std::string>()))
                throw std::invalid_argument("Missing argument: " + required.get<std::string>());
        for (auto it = args.begin(); it != args.end(); ++it) {
            if (!schema["properties"].contains(it.key()))
                throw std::invalid_argument("Unknown argument: " + it.key());
            validate(it.value(), schema["properties"][it.key()], it.key());
        }
        for (auto it = schema["properties"].begin(); it != schema["properties"].end(); ++it)
            if (!args.contains(it.key()) && it.value().contains("default"))
                args[it.key()] = it.value()["default"];
        return tool.invoke(args);
    }
} // namespace snt::mcp
