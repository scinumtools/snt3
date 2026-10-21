#ifndef SNT_DIP_TOOLS_VALIDATOR_H
#define SNT_DIP_TOOLS_VALIDATOR_H

#include <filesystem>
#include <snt/core/diagnostic.h>
#include <string>
#include <string_view>
#include <vector>

namespace snt::dip::tools {

    /** Limits for diagnostic collection without changing validity rules. */
    struct ValidationOptions {
        int max_syntax_errors = 100; ///< Maximum independent syntax errors reported; 1 to 1000.
    };

    /** Diagnostics produced while validating one DIPL input. */
    struct ValidationResult {
        std::vector<core::Diagnostic> diagnostics; ///< Empty when the input is valid.

        /** Return true when parsing and evaluation completed without an error. */
        bool valid() const noexcept { return diagnostics.empty(); }
    };

    /** Validate in-memory DIPL using the ordinary C++ parser and evaluator. */
    ValidationResult validate_text(
        std::string_view source, std::string source_name = "<memory>", const ValidationOptions& options = {}
    );

    /** Validate a DIPL file or DIPfile project without writing output files.
     * A file named DIPfile is treated as a project; project=true also accepts
     * a manifest with another filename. Independent statement syntax errors
     * are collected; semantic evaluation remains fail-fast. The nearest parent
     * DIPfile supplies max_syntax_errors for a standalone source file.
     */
    ValidationResult validate_file(const std::filesystem::path& input, bool project = false);

} // namespace snt::dip::tools

#endif
