#include <snt/dip/inspect/diagnostic.h>

#include <snt/dip/exceptions.h>

namespace snt::dip {
namespace {
std::string category(const dip::Exception& error) {
    if (dynamic_cast<const SyntaxException*>(&error)) return "dip.syntax";
    if (dynamic_cast<const ParserException*>(&error)) return "dip.parser";
    if (dynamic_cast<const SolverException*>(&error)) return "dip.solver";
    if (dynamic_cast<const UnitException*>(&error)) return "dip.unit";
    if (dynamic_cast<const IOException*>(&error)) return "dip.io";
    if (dynamic_cast<const EnvironmentException*>(&error)) return "dip.environment";
    if (dynamic_cast<const PybindException*>(&error)) return "dip.pybind";
    if (dynamic_cast<const MissingException*>(&error)) return "dip.missing";
    return "dip.error";
}
} // namespace

core::Diagnostic diagnostic_from_exception(const std::exception& error) {
    if (const auto* dip_error = dynamic_cast<const dip::Exception*>(&error)) {
        const auto& info = dip_error->info();
        return {core::DiagnosticSeverity::Error, category(*dip_error), info.message,
                info.details, info.suggestion, std::nullopt, info.location, info.origin};
    }
    if (const auto* core_error = dynamic_cast<const core::Exception*>(&error)) {
        const auto& info = core_error->info();
        return {core::DiagnosticSeverity::Error, "core.error", info.message,
                info.details, info.suggestion, std::nullopt, std::nullopt, info.origin};
    }
    return {core::DiagnosticSeverity::Error, "dip.error", error.what()};
}

} // namespace snt::dip
