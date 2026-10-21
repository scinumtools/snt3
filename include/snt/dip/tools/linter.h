#ifndef SNT_DIP_TOOLS_LINTER_H
#define SNT_DIP_TOOLS_LINTER_H

#include <filesystem>
#include <snt/core/diagnostic.h>
#include <snt/dip/environment.h>
#include <string>
#include <string_view>
#include <vector>

namespace snt::dip::tools {

    /** Advisory checks applied only after DIPL has been parsed and evaluated. */
    struct LintOptions {
        bool require_public_descriptions = true;      ///< Warn when public values or schemas lack ?descr.
        bool warn_deprecated_values = true;           ///< Warn when an evaluated value carries ?deprecated.
        bool warn_duplicate_options = true;           ///< Warn when a choice occurs more than once in !options.
        bool require_public_option_labels = true;     ///< Warn when public numeric choices lack !labels.
        bool warn_broken_metadata_references = false; ///< Check path-like advisory metadata against evaluated paths.
        bool warn_unused_schemas = false;             ///< Warn about definitions with no evaluated application.
    };

    /** Validation errors or advisory findings for one input. */
    struct LintResult {
        bool input_valid = true;                   ///< False when validation prevented linting.
        std::vector<core::Diagnostic> diagnostics; ///< Errors or lint warnings with source locations.

        /** True when the input is valid and no enabled rule reported a finding. */
        bool clean() const noexcept { return input_valid && diagnostics.empty(); }
    };

    /** Lint an already evaluated environment without reparsing or modifying it. */
    LintResult lint_environment(const Environment& environment, const LintOptions& options = {});

    /** Validate and lint in-memory DIPL. Invalid input reports validation errors only. */
    LintResult lint_text(
        std::string_view source, std::string source_name = "<memory>", const LintOptions& options = {}
    );

    /** Validate and lint a source file or DIPfile project. The nearest DIPfile supplies settings. */
    LintResult lint_file(const std::filesystem::path& input, bool project = false);

} // namespace snt::dip::tools

#endif
