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
        std::optional<std::string> code = std::nullopt;
        std::optional<std::size_t> column = std::nullopt;
    };

    struct ExceptionInfo {
        std::string message;                  ///< Exception message
        std::string expected;                 ///< Expected value, behavior
        std::string actual;                   ///< Actual value, behavior
        std::string suggestion;               ///< Suggested changes
        std::optional<SourceLocation> origin; ///< Where the error was thrown
    };

    template <typename E> class ExceptionBase : public std::runtime_error {
      public:
        explicit ExceptionBase(std::string message, std::string prefix = "[SNT-CORE] ")
            : std::runtime_error(message), m_info({std::move(message)}) {
            m_what = format(m_info, prefix);
        }
        explicit ExceptionBase(E info, std::string prefix = "[SNT-CORE] ")
            : std::runtime_error(info.message), m_info(std::move(info)) {
            m_what = format(m_info, prefix);
        }

        const char* what() const noexcept override { return m_what.c_str(); }

        const E& info() const noexcept { return m_info; }

      protected:
        virtual std::string format(const E& info, std::string prefix = "") {
            std::ostringstream out;
            out << prefix << info.message;
            if (!info.expected.empty())
                out << "\n  expected:   " << info.expected;
            if (!info.actual.empty())
                out << "\n  actual:     " << info.actual;
            if (!info.suggestion.empty())
                out << "\n  suggestion: " << info.suggestion;
            if (info.origin)
                out << "\n  thrown:     " << format_location(*info.origin);
            return out.str();
        }
        std::string format_location(const SourceLocation& location) {
            std::ostringstream out;
            out << location.source;
            if (location.line != 0) {
                out << ':' << location.line;
                if (location.column) {
                    out << ':' << *location.column;
                }
            }
            if (location.code)
                out << " | " << *location.code;
            return out.str();
        }

      protected:
        E m_info;
        std::string m_what;
    };

    using Exception = core::ExceptionBase<core::ExceptionInfo>;

    /**
     * Missing exception is triggered when functionality is not implemented yet
     */
    class MissingException : public core::Exception {
      public:
        MissingException(std::string message, std::string file, std::size_t line)
            : core::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      "",
                      "",
                      "If you need this functionality, please contact the developers, open an issue or send a pull "
                      "request on GitHub.",
                      core::SourceLocation{file, line}
                  }
              ) {};
    };

} // namespace snt::core

#endif // SNT_CORE_EXCEPTIONS_H
