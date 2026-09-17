#ifndef SNT_CORE_EXCEPTIONS_H
#define SNT_CORE_EXCEPTIONS_H

#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace snt::core {

    /** Source location associated with a diagnostic. */
    struct SourceLocation {
        std::string source;   ///< Source file or input name.
        std::size_t line = 0; ///< One-based source line, or zero when unavailable.
        /// Source text associated with the location.
        std::optional<std::string> code = std::nullopt;
        /// Optional one-based source column.
        std::optional<std::size_t> column = std::nullopt;
    };

    /** Structured information carried by an SNT exception. */
    struct ExceptionInfo {
        std::string message;                  ///< Exception message
        std::string details;                  ///< Details error description
        std::string suggestion;               ///< Suggested changes
        std::optional<SourceLocation> origin; ///< Where the error was thrown in the source
    };

    template <typename E> class ExceptionBase : public std::runtime_error {
      public:
        /** Construct an exception from structured information and an optional display prefix. */
        explicit ExceptionBase(E info, std::string prefix = "[SNT-CORE] ")
            : std::runtime_error(info.message), m_info(std::move(info)) {
            m_what = format(m_info, prefix);
        }

        /** Return the fully formatted diagnostic text. */
        const char* what() const noexcept override { return m_what.c_str(); }

        /** Return the structured exception information. */
        const E& info() const noexcept { return m_info; }

      protected:
        /**
         * Add specific indent in front of every new line
         */
        std::string indent_lines(const std::string& str) {
            std::string result = str;
            std::size_t pos = 0;
            std::string indent(14, ' ');
            while ((pos = result.find('\n', pos)) != std::string::npos) {
                result.insert(pos + 1, indent);
                pos += 1 + indent.size();
            }
            return result;
        }

        /** Format structured information into the text returned by what(). */
        virtual std::string format(const E& info, std::string prefix = "") {
            std::ostringstream out;
            out << prefix << info.message;
            if (!info.details.empty())
                out << "\n  details:    " << indent_lines(info.details);
            if (!info.suggestion.empty())
                out << "\n  suggestion: " << indent_lines(info.suggestion);
            if (info.origin)
                out << "\n  source:     " << format_location(*info.origin);
            return out.str();
        }
        /** Format a source location for inclusion in a diagnostic. */
        std::string format_location(const SourceLocation& location) {
            std::ostringstream out;
            out << location.source;
            if (location.line != 0) {
                out << ':' << location.line;
                if (location.column) {
                    out << ':' << *location.column;
                }
            }
            if (location.code && !location.code->empty())
                out << " | " << *location.code;
            return out.str();
        }

      protected:
        E m_info;
        std::string m_what;
    };

    // using Exception = core::ExceptionBase<core::ExceptionInfo>;

    /** Base exception type for errors raised by the core library. */
    class Exception : public ExceptionBase<ExceptionInfo> {
      public:
        /** Make the base exception constructors available. */
        using ExceptionBase<ExceptionInfo>::ExceptionBase;

        /** Construct an exception with message, details, suggestion, and source location. */
        Exception(
            std::string message,
            std::string details,
            std::string suggestion,
            std::string origin_file,
            size_t origin_line
        )
            : ExceptionBase(
                  ExceptionInfo{
                      std::move(message),
                      std::move(details),
                      std::move(suggestion),
                      SourceLocation{std::move(origin_file), origin_line}
                  }
              ) {}
    };

    /**
     * Parse exceptions are triggered during parsing
     */
    /** Exception raised while parsing input. */
    class ParserException : public core::Exception {
      public:
        /** Make the base exception constructors available. */
        using core::Exception::Exception;
    };

    /**
     * Missing exception is triggered when functionality is not implemented yet
     */
    /** Exception raised when requested functionality is unavailable. */
    class MissingException : public core::Exception {
      public:
        /** Construct an exception identifying unavailable functionality. */
        MissingException(std::string message, std::string file, std::size_t line)
            : core::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      "",
                      "If you need this functionality, please contact the developers, open an issue or send a pull "
                      "request on GitHub.",
                      core::SourceLocation{file, line}
                  }
              ) {};
    };

} // namespace snt::core

#endif // SNT_CORE_EXCEPTIONS_H
