#ifndef SNT_CORE_DIAGNOSTIC_H
#define SNT_CORE_DIAGNOSTIC_H

#include <snt/core/exceptions.h>

#include <optional>
#include <string>

namespace snt::core {

enum class DiagnosticSeverity { Info, Warning, Error };

/** Structured diagnostic for CLI, tests, and interactive clients. */
struct Diagnostic {
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::string code; ///< Stable category, such as "dip.syntax".
    std::string message;
    std::string details;
    std::string suggestion;
    std::optional<std::string> node_path;
    std::optional<SourceLocation> location; ///< User input location, when known.
    std::optional<SourceLocation> origin; ///< Implementation location, when known.
};

} // namespace snt::core

#endif
