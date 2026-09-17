#ifndef SNT_API_EXCEPTIONS_H
#define SNT_API_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::api {

    /** Base exception for the application-facing SNT API. */
    class Exception : public core::Exception {
      public:
        /** Construct an API exception from structured exception information. */
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-API] ") {}

        /** Construct an API exception from message, diagnostics, and source location.
         *  @param message Short error message.
         *  @param details Detailed explanation of the failure.
         *  @param suggestion Suggested corrective action.
         *  @param origin_file Source file where the error originated.
         *  @param origin_line Source line where the error originated.
         */
        explicit Exception(
            std::string message,
            std::string details,
            std::string suggestion,
            std::string origin_file,
            size_t origin_line
        )
            : api::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      std::move(details),
                      std::move(suggestion),
                      core::SourceLocation{std::move(origin_file), origin_line}
                  }
              ) {}
    };

    /** Exception raised when API arguments are invalid or inconsistent. */
    class ArgumentException : public api::Exception {
      public:
        /** Inherit the base exception constructors. */
        using api::Exception::Exception;
    };

    /** Exception raised when requested API functionality is unavailable. */
    class MissingException : public api::Exception {
      public:
        /** Construct an exception for unavailable functionality.
         *  @param message Description of the unavailable operation.
         *  @param file Source file where the exception was raised.
         *  @param line Source line where the exception was raised.
         */
        MissingException(std::string message, std::string file, std::size_t line)
            : api::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      "",
                      "If you need this functionality, please contact the developers, open an issue or send a pull "
                      "request on GitHub.",
                      core::SourceLocation{std::move(file), line}
                  }
              ) {};
    };

} // namespace snt::api

#endif // SNT_API_EXCEPTIONS_H
