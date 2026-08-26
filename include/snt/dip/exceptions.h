#ifndef SNT_DIP_EXCEPTIONS_H
#define SNT_DIP_EXCEPTIONS_H

#include <snt/core/exceptions.h>
#include <snt/dip/settings.h>

namespace snt::dip {

    struct ExceptionInfo : public core::ExceptionInfo {
        std::optional<core::SourceLocation> location; ///< DIPL code line where exception occured
    };

    class Exception : public core::ExceptionBase<dip::ExceptionInfo> {
      public:
        explicit Exception(std::string message)
            : core::ExceptionBase<dip::ExceptionInfo>(std::move(message), "[SNT-DIP] ") {
            m_what = format(m_info, "[SNT-DIP] ");
        }
        explicit Exception(dip::ExceptionInfo info)
            : core::ExceptionBase<dip::ExceptionInfo>(std::move(info), "[SNT-DIP] ") {
            m_what = format(m_info, "[SNT-DIP] ");
        }
        explicit Exception(
            std::string message,
            std::string expected,
            std::string actual,
            std::string suggestion,
            std::string origin_file,
            size_t origin_line,
            std::optional<Line> location_line = std::nullopt
        )
            : dip::Exception(
                  dip::ExceptionInfo{
                      std::move(message),
                      std::move(expected),
                      std::move(actual),
                      std::move(suggestion),
                      core::SourceLocation{std::move(origin_file), origin_line},
                      location_line
                          ? std::make_optional<core::SourceLocation>(core::SourceLocation{
                                location_line->source.name, location_line->source.line_number, location_line->code
                            })
                          : std::nullopt
                  }
              ) {}

      protected:
        std::string format(const dip::ExceptionInfo& info, std::string prefix = "") override {
            std::ostringstream out;
            out << prefix << info.message;
            if (!info.expected.empty())
                out << "\n  expected:   " << info.expected;
            if (!info.actual.empty())
                out << "\n  actual:     " << info.actual;
            if (!info.suggestion.empty())
                out << "\n  suggestion: " << info.suggestion;
            if (info.location) {
                out << "\n    at:       " << format_location(*info.location);
            }
            if (info.origin)
                out << "\n  thrown:     " << format_location(*info.origin);
            return out.str();
        }
    };

    /**
     * Parser exceptions occurs when internal DIP parsers fails
     */
    class ParserException : public dip::Exception {
      public:
        using dip::Exception::Exception;
    };

    /**
     * Solver exceptions occurs when solver cannot process an expression
     */
    class SolverException : public dip::Exception {
      public:
        using dip::Exception::Exception;
    };

    /**
     * In/Out exceptions is triggered by problems with a file system (e.g. missing files, timeouts, ...)
     */
    class IOException : public dip::Exception {
      public:
        using dip::Exception::Exception;
    };

    /**
     * Environment exceptions signal problems with DIP environments
     */
    class EnvironmentException : public dip::Exception {
      public:
        using dip::Exception::Exception;
    };

    /**
     * Unit exceptions indicate problems with units, conversions and operations
     */
    class UnitException : public dip::Exception {
      public:
        using dip::Exception::Exception;
    };

    /**
     * Syntax exceptions are triggered by incorrect DIPL syntax
     */
    class SyntaxException : public dip::Exception {
      public:
        using dip::Exception::Exception;
    };
} // namespace snt::dip

#endif // SNT_DIP_EXCEPTIONS_H
