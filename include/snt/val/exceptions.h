#ifndef SNT_VAL_EXCEPTIONS_H
#define SNT_VAL_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::val {

    class Exception : public core::Exception {
      public:
        explicit Exception(std::string message) : core::Exception(std::move(message), "[SNT-VAL] ") {}
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-VAL] ") {}
        explicit Exception(
            std::string message,
            std::string details,
            std::string suggestion,
            std::string origin_file,
            size_t origin_line
        )
            : val::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      std::move(details),
                      std::move(suggestion),
                      core::SourceLocation{std::move(origin_file), origin_line}
                  }
              ) {}
    };

    /**
     * Type exceptions are triggered by type related errors
     */
    class TypeException : public val::Exception {
      public:
        using val::Exception::Exception;
    };

    /**
     * Array exceptions are triggered by inconsitencies with arrays
     */
    class ArrayException : public val::Exception {
      public:
        using val::Exception::Exception;
    };

    /**
     * Missing exception is triggered when functionality is not implemented yet
     */
    class MissingException : public val::Exception {
      public:
        MissingException(std::string message, std::string file, std::size_t line)
            : val::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      "",
                      "If you need this functionality, please contact the developers, open an issue or send a pull "
                      "request on GitHub.",
                      core::SourceLocation{file, line}
                  }
              ) {};
    };

} // namespace snt::val

#endif // SNT_VAL_EXCEPTIONS_H
