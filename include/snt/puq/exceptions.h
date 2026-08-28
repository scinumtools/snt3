#ifndef SNT_PUQ_EXCEPTIONS_H
#define SNT_PUQ_EXCEPTIONS_H

#include <snt/core/exceptions.h>
#include <snt/puq/systems/unit_system.h>
#include <string>

namespace snt::puq {

    class Exception : public core::Exception {
      public:
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-PUQ] ") {}
        explicit Exception(
            std::string message,
            std::string details,
            std::string suggestion,
            std::string origin_file,
            size_t origin_line
        )
            : puq::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      std::move(details),
                      std::move(suggestion),
                      core::SourceLocation{std::move(origin_file), origin_line}
                  }
              ) {}
    };

    /**
     * Parser exceptions occurs when internal PUQ parsers fails
     */
    class ParserException : public puq::Exception {
      public:
        using puq::Exception::Exception;
    };

    /**
     * Unit exceptions indicate problems with units, conversions and operations
     */
    class UnitException : public puq::Exception {
      public:
        using puq::Exception::Exception;
    };

    /**
     * Unit system exceptions indicate problems with unit systems
     */
    class SystemException : public puq::Exception {
      public:
        using puq::Exception::Exception;
    };

    /**
     * Missing exception is triggered when functionality is not implemented yet
     */
    class MissingException : public puq::Exception {
      public:
        MissingException(std::string message, std::string file, std::size_t line)
            : puq::Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      "",
                      "If you need this functionality, please contact the developers, open an issue or send a pull "
                      "request on GitHub.",
                      core::SourceLocation{file, line}
                  }
              ) {};
    };

} // namespace snt::puq

#endif // SNT_PUQ_EXCEPTIONS_H
