#include <iomanip>
#include <snt/api/dip_validate.h>
#include <sstream>
#include <string_view>

namespace snt::api {
    namespace {
        std::string quote(std::string_view value) {
            std::ostringstream out;
            out << '"';
            for (unsigned char c : value) {
                switch (c) {
                case '"':
                    out << "\\\"";
                    break;
                case '\\':
                    out << "\\\\";
                    break;
                case '\b':
                    out << "\\b";
                    break;
                case '\f':
                    out << "\\f";
                    break;
                case '\n':
                    out << "\\n";
                    break;
                case '\r':
                    out << "\\r";
                    break;
                case '\t':
                    out << "\\t";
                    break;
                default:
                    if (c < 0x20)
                        out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << int(c) << std::dec;
                    else
                        out << char(c);
                }
            }
            out << '"';
            return out.str();
        }

        void location(std::ostream& out, const std::optional<core::SourceLocation>& value) {
            if (!value) {
                out << "null";
                return;
            }
            out << "{\"source\":" << quote(value->source) << ",\"line\":" << value->line << ",\"column\":";
            if (value->column)
                out << *value->column;
            else
                out << "null";
            out << '}';
        }

        const char* severity(core::DiagnosticSeverity value) {
            switch (value) {
            case core::DiagnosticSeverity::Info:
                return "info";
            case core::DiagnosticSeverity::Warning:
                return "warning";
            case core::DiagnosticSeverity::Error:
                return "error";
            }
            return "error";
        }
    } // namespace

    dip::tools::ValidationResult validate_dip_file(const std::filesystem::path& input, bool project) {
        return dip::tools::validate_file(input, project);
    }

    std::string dip_validation_json(const std::filesystem::path& input, const dip::tools::ValidationResult& result) {
        return dip_diagnostics_json(input, result.valid(), result.diagnostics);
    }

    std::string dip_diagnostics_json(
        const std::filesystem::path& input, bool valid, const std::vector<core::Diagnostic>& diagnostics,
        std::optional<bool> clean
    ) {
        std::ostringstream out;
        out << "{\"input\":" << quote(input.generic_string()) << ",\"valid\":" << (valid ? "true" : "false");
        if (clean)
            out << ",\"clean\":" << (*clean ? "true" : "false");
        out << ",\"diagnostics\":[";
        for (std::size_t i = 0; i < diagnostics.size(); ++i) {
            if (i)
                out << ',';
            const auto& diagnostic = diagnostics[i];
            out << "{\"severity\":" << quote(severity(diagnostic.severity)) << ",\"code\":" << quote(diagnostic.code)
                << ",\"message\":" << quote(diagnostic.message) << ",\"details\":" << quote(diagnostic.details)
                << ",\"suggestion\":" << quote(diagnostic.suggestion) << ",\"node_path\":";
            if (diagnostic.node_path)
                out << quote(*diagnostic.node_path);
            else
                out << "null";
            out << ",\"location\":";
            location(out, diagnostic.location);
            out << '}';
        }
        out << "]}";
        return out.str();
    }

} // namespace snt::api
