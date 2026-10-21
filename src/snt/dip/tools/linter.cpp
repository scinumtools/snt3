#include <algorithm>
#include <cctype>
#include <map>
#include <optional>
#include <set>
#include <snt/dip/dip.h>
#include <snt/dip/inspect/diagnostic.h>
#include <snt/dip/inspect/inspector.h>
#include <snt/dip/nodes/path.h>
#include <snt/dip/tools/linter.h>
#include <snt/dip/tools/validator.h>
#include <utility>

namespace snt::dip::tools {
    namespace {
        core::Diagnostic warning(
            std::string code,
            std::string message,
            std::string suggestion,
            const core::SourceLocation& location,
            std::string path
        ) {
            core::Diagnostic diagnostic;
            diagnostic.severity = core::DiagnosticSeverity::Warning;
            diagnostic.code = std::move(code);
            diagnostic.message = std::move(message);
            diagnostic.suggestion = std::move(suggestion);
            diagnostic.location = location;
            diagnostic.node_path = std::move(path);
            return diagnostic;
        }

        LintResult invalid(ValidationResult validation) {
            LintResult result;
            result.input_valid = false;
            result.diagnostics = std::move(validation.diagnostics);
            return result;
        }

        core::SourceLocation physical_location(
            const core::SourceLocation& origin, const std::map<std::string, SourceInfo>& sources
        ) {
            auto location = origin;
            const auto source = sources.find(origin.source);
            if (source != sources.end() && !source->second.path.empty())
                location.source = source->second.path;
            return location;
        }

        bool numeric_type(const std::string& type) {
            return type.compare(0, 3, "int") == 0 || type.compare(0, 4, "uint") == 0 ||
                   type.compare(0, 4, "sint") == 0 || type.compare(0, 5, "float") == 0;
        }

        // Advisory metadata can also contain prose or external identifiers.
        // A query marker or a dotted/collection path makes local intent clear.
        std::optional<std::string> local_metadata_path(std::string_view text) {
            const bool explicit_query = !text.empty() && text.front() == '?';
            if (explicit_query)
                text.remove_prefix(1);
            if (text.empty() || (!explicit_query && text.find_first_of(".[") == std::string_view::npos) ||
                text.back() == '.' || text.find("..") != std::string_view::npos)
                return std::nullopt;
            for (const unsigned char c : text) {
                if (!std::isalnum(c) && c != '_' && c != '-' && c != '.' && c != '[' && c != ']')
                    return std::nullopt;
            }
            try {
                const Path parsed{std::string(text)};
                if (parsed.kind != Path::Kind::Root && parsed.kind != Path::Kind::None)
                    return parsed.name;
            } catch (const std::exception&) {
                // Text resembling a path is still advisory prose if it is not DIPL path syntax.
            }
            return std::nullopt;
        }
    } // namespace

    LintResult lint_environment(const Environment& environment, const LintOptions& options) {
        LintResult result;
        const Inspector inspector(environment);

        // describe(..., 0) retains the semantic facts needed for linting but
        // does not stringify or copy potentially large parameter values.
        for (const auto& path : inspector.select_paths()) {
            const auto value = inspector.describe(path, 0);
            if (!value.declaration)
                continue;
            if (options.require_public_descriptions && value.metadata.visibility == "public" &&
                value.metadata.description.empty()) {
                result.diagnostics.push_back(warning(
                    "dip.lint.public-description",
                    "Public value has no description",
                    "Add a nonempty ?descr to this value or its schema member.",
                    *value.declaration,
                    path
                ));
            }
            if (options.warn_deprecated_values && !value.metadata.deprecated.empty()) {
                auto suggestion = std::string("Review the ?deprecated note before using this value.");
                if (!value.metadata.replacement.empty())
                    suggestion = "Use `" + value.metadata.replacement + "` instead.";
                auto diagnostic = warning(
                    "dip.lint.deprecated-value",
                    "Evaluated value is deprecated",
                    std::move(suggestion),
                    *value.declaration,
                    path
                );
                diagnostic.details = value.metadata.deprecated;
                result.diagnostics.push_back(std::move(diagnostic));
            }

            if (options.warn_duplicate_options && value.enforced_options.size() > 1) {
                std::set<std::string> seen;
                std::set<std::string> repeated;
                for (const auto& option : value.enforced_options)
                    if (!seen.insert(option).second)
                        repeated.insert(option);
                if (!repeated.empty()) {
                    auto diagnostic = warning(
                        "dip.lint.duplicate-options",
                        "Choice list contains duplicate options",
                        "Remove repeated values from !options or make the choices distinct.",
                        *value.declaration,
                        path
                    );
                    for (const auto& option : repeated) {
                        if (!diagnostic.details.empty())
                            diagnostic.details += ", ";
                        diagnostic.details += "`" + option + "`";
                    }
                    result.diagnostics.push_back(std::move(diagnostic));
                }
            }
            if (options.require_public_option_labels && value.metadata.visibility == "public" &&
                numeric_type(value.declared_type) && value.enforced_options.size() > 1 &&
                (value.option_labels.empty() ||
                 std::any_of(value.option_labels.begin(), value.option_labels.end(), [](const std::string& label) {
                     return label.empty();
                 }))) {
                result.diagnostics.push_back(warning(
                    "dip.lint.public-option-labels",
                    "Public numeric choices have missing labels",
                    "Add one nonempty !labels entry for each !options choice.",
                    *value.declaration,
                    path
                ));
            }
            if (options.warn_broken_metadata_references) {
                // Only path-like strings are checked. Bare words and prose may
                // describe external concepts, so they are deliberately ignored.
                const auto check_reference = [&](std::string_view field, std::string_view text) {
                    const auto target = local_metadata_path(text);
                    if (!target)
                        return;
                    try {
                        (void)inspector.capabilities(*target);
                    } catch (const std::exception&) {
                        auto diagnostic = warning(
                            "dip.lint.broken-metadata-reference",
                            "Metadata references an unavailable path",
                            "Correct the local path or disable this optional rule for external references.",
                            *value.declaration,
                            path
                        );
                        diagnostic.details = std::string(field) + " references `" + *target + "`.";
                        result.diagnostics.push_back(std::move(diagnostic));
                    }
                };
                check_reference("?replacement", value.metadata.replacement);
                for (const auto& target : value.metadata.requires)
                    check_reference("?requires", target);
                for (const auto& target : value.metadata.conflicts)
                    check_reference("?conflicts", target);
                for (const auto& target : value.metadata.implies)
                    check_reference("?implies", target);
            }
        }

        // The hierarchy exposes registered definitions and actual application
        // events, so the optional unused rule does not need to infer usage.
        if (options.require_public_descriptions || options.warn_unused_schemas) {
            const auto hierarchy = inspector.schema_hierarchy();
            std::set<std::string> applied;
            if (options.warn_unused_schemas && hierarchy.applications_complete) {
                for (const auto& application : hierarchy.applications)
                    applied.insert(application.schema_ids.begin(), application.schema_ids.end());
            }
            for (const auto& definition : hierarchy.definitions) {
                const auto location = physical_location(definition.origin, hierarchy.sources);
                if (options.require_public_descriptions && definition.metadata.visibility == "public" &&
                    definition.metadata.description.empty()) {
                    result.diagnostics.push_back(warning(
                        "dip.lint.public-description",
                        "Public schema has no description",
                        "Add a nonempty ?descr to the schema definition.",
                        location,
                        definition.id
                    ));
                }
                if (options.warn_unused_schemas && hierarchy.applications_complete &&
                    applied.find(definition.id) == applied.end()) {
                    result.diagnostics.push_back(warning(
                        "dip.lint.unused-schema",
                        "Schema has no evaluated application",
                        "Apply the schema or disable this rule for a reusable schema library.",
                        location,
                        definition.id
                    ));
                }
            }
        }
        return result;
    }

    LintResult lint_text(std::string_view source, std::string source_name, const LintOptions& options) {
        auto validation = validate_text(source, source_name);
        if (!validation.valid())
            return invalid(std::move(validation));
        try {
            DIP parser;
            parser.add_string(std::string(source), source_name);
            return lint_environment(parser.parse(), options);
        } catch (const std::exception& error) {
            return invalid({{diagnostic_from_exception(error)}});
        }
    }

    LintResult lint_file(const std::filesystem::path& input, bool project) {
        auto validation = validate_file(input, project);
        if (!validation.valid())
            return invalid(std::move(validation));
        try {
            const auto absolute = std::filesystem::absolute(input).lexically_normal();
            LintOptions options;
            const auto manifest = (project || absolute.filename() == "DIPfile")
                                      ? std::optional<std::filesystem::path>(absolute)
                                      : nearest_project_manifest(absolute);
            if (manifest) {
                const auto settings = parse_project_manifest(*manifest, {}, manifest->string());
                options.require_public_descriptions = settings["lint.require_public_descriptions"].as<bool>();
                options.warn_deprecated_values = settings["lint.warn_deprecated_values"].as<bool>();
                options.warn_duplicate_options = settings["lint.warn_duplicate_options"].as<bool>();
                options.require_public_option_labels = settings["lint.require_public_option_labels"].as<bool>();
                options.warn_broken_metadata_references = settings["lint.warn_broken_metadata_references"].as<bool>();
                options.warn_unused_schemas = settings["lint.warn_unused_schemas"].as<bool>();
            }
            DIP parser;
            if (project || absolute.filename() == "DIPfile")
                parser.add_project(absolute);
            else
                parser.add_file(absolute, absolute.string(), true);
            return lint_environment(parser.parse(), options);
        } catch (const std::exception& error) {
            return invalid({{diagnostic_from_exception(error)}});
        }
    }

} // namespace snt::dip::tools
