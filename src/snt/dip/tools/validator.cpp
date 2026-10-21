#include <exception>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/inspect/diagnostic.h>
#include <snt/dip/tools/validator.h>
#include <stdexcept>
#include <utility>

namespace snt::dip::tools {
    namespace {
        template <typename Run> ValidationResult validate(Run&& run) {
            ValidationResult result;
            try {
                run(result);
            } catch (const std::exception& error) {
                // Registration and semantic evaluation remain fail-fast.
                result.diagnostics.push_back(diagnostic_from_exception(error));
            }
            return result;
        }

        void parse_if_syntax_valid(DIP& parser, ValidationResult& result, const ValidationOptions& options) {
            if (options.max_syntax_errors < 1 || options.max_syntax_errors > 1000)
                throw std::invalid_argument("validate.max_syntax_errors must be between 1 and 1000");
            result.diagnostics = parser.collect_syntax_diagnostics(static_cast<std::size_t>(options.max_syntax_errors));
            if (result.valid())
                (void)parser.parse();
        }
    } // namespace

    ValidationResult validate_text(std::string_view source, std::string source_name, const ValidationOptions& options) {
        return validate([&](ValidationResult& result) {
            DIP parser;
            parser.add_string(std::string(source), source_name);
            parse_if_syntax_valid(parser, result, options);
        });
    }

    ValidationResult validate_file(const std::filesystem::path& input, bool project) {
        return validate([&](ValidationResult& result) {
            const auto absolute = std::filesystem::absolute(input).lexically_normal();
            DIP parser;
            ValidationOptions options;
            if (project || absolute.filename() == "DIPfile") {
                // Check the manifest before add_project() evaluates it. This
                // also lets us report several syntax mistakes in the DIPfile.
                DIP manifest;
                manifest.add_file(absolute, absolute.string(), true);
                result.diagnostics = manifest.collect_syntax_diagnostics();
                if (!result.valid())
                    return;
                const auto manifest_environment = parse_project_manifest(absolute, {}, absolute.string());
                options.max_syntax_errors = manifest_environment["validate.max_syntax_errors"].as<int>();
                parser.add_project(absolute);
            } else {
                if (const auto manifest = nearest_project_manifest(absolute)) {
                    const auto manifest_environment = parse_project_manifest(*manifest, {}, manifest->string());
                    options.max_syntax_errors = manifest_environment["validate.max_syntax_errors"].as<int>();
                }
                parser.add_file(absolute, absolute.string(), true);
            }
            parse_if_syntax_valid(parser, result, options);
        });
    }

} // namespace snt::dip::tools
