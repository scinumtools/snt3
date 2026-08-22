#ifndef SNT_CORE_EXCEPTIONS_H
#define SNT_CORE_EXCEPTIONS_H

#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace snt::core {

    struct SourceLocation {
        std::string source;
        std::size_t line = 0;
        std::string code;
        std::optional<std::size_t> column;
    };

    struct ExceptionInfo {
        std::string message;                    ///< Exception message
        std::string expected;                   ///< Expected value, behavior
        std::string actual;                     ///< Actual value, behavior
        std::string suggestion;                 ///< Suggested changes
        std::optional<SourceLocation> location; ///< Where the error is reported to the user.
        std::optional<SourceLocation> origin;   ///< Where the operation originated, e.g. DIPL.
    };

    class Exception : public std::runtime_error {
      public:
        explicit Exception(std::string message, std::string prefix = "[SNT-CORE] ")
            : std::runtime_error(message), m_info(ExceptionInfo({message})), m_what(format(m_info, prefix)) {}
        explicit Exception(ExceptionInfo info, std::string prefix = "[SNT-CORE] ")
            : std::runtime_error(info.message), m_info(std::move(info)), m_what(format(m_info, prefix)) {}

        const char* what() const noexcept override { return m_what.c_str(); }

        const ExceptionInfo& info() const noexcept { return m_info; }

      private:
        static std::string format(const ExceptionInfo& info, std::string prefix = "") {
            std::ostringstream out;
            out << prefix << info.message;
            if (!info.expected.empty())
                out << "\n  expected:   " << info.expected;
            if (!info.actual.empty())
                out << "\n  actual:     " << info.actual;
            if (!info.suggestion.empty())
                out << "\n  suggestion: " << info.suggestion;
            if (info.origin) {
                out << "\n  at:         " << format_location(*info.location);
            }
            if (info.location)
                out << "\n  thrown:     " << format_location(*info.origin);
            return out.str();
        }
        static std::string format_location(const SourceLocation& location) {
            std::ostringstream out;
            out << location.source;
            if (location.line != 0) {
                out << ':' << location.line;
                if (location.column) {
                    out << ':' << *location.column;
                }
            }
            if (!location.code.empty())
                out << " | " << location.code;
            return out.str();
        }

      private:
        ExceptionInfo m_info;
        std::string m_what;
    };

} // namespace snt::core

#endif // SNT_CORE_EXCEPTIONS_H
