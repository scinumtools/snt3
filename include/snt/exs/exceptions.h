#ifndef SNT_EXS_EXCEPTIONS_H
#define SNT_EXS_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::exs {

    class Exception : public core::Exception {
      public:
        explicit Exception(std::string message) : core::Exception(std::move(message), "[SNT-EXS] ") {}
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-EXS] ") {}
        explicit Exception(
            std::string message,
            std::string details,
            std::string suggestion,
            std::string origin_file,
            size_t origin_line
        )
            : exs::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      std::move(details),
                      std::move(suggestion),
                      core::SourceLocation{std::move(origin_file), origin_line}
                  }
              ) {}
    };

    /**
     * Parser exceptions are triggered by problems during expression parsing
     */
    class ParserException : public exs::Exception {
      public:
        using exs::Exception::Exception;
    };

    /**
     * Missing exception is triggered when functionality is not implemented yet
     */
    class MissingException : public exs::Exception {
      public:
        MissingException(std::string message, std::string file, std::size_t line)
            : exs::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      "",
                      "If you need this functionality, please contact the developers, open an issue or send a pull "
                      "request on GitHub.",
                      core::SourceLocation{file, line}
                  }
              ) {};
    };

} // namespace snt::exs

#endif // SNT_EXS_EXCEPTIONS_H
