#ifndef SNT_API_EXCEPTIONS_H
#define SNT_API_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::api {

    class Exception : public core::Exception {
      public:
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-API] ") {}
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

    /**
     * Argument exceptions indicate problems with input aguments
     */
    class ArgumentException : public api::Exception {
      public:
        using api::Exception::Exception;
    };

    /**
     * Missing exception is triggered when functionality is not implemented yet
     */
    class MissingException : public api::Exception {
      public:
        MissingException(std::string message, std::string file, std::size_t line)
            : api::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      "",
                      "If you need this functionality, please contact the developers, open an issue or send a pull "
                      "request on GitHub.",
                      core::SourceLocation{file, line}
                  }
              ) {};
    };

} // namespace snt::api

#endif // SNT_API_EXCEPTIONS_H
