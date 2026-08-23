#ifndef SNT_EXS_EXCEPTIONS_H
#define SNT_EXS_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::exs {

    class Exception : public core::Exception {
      public:
        explicit Exception(std::string message) : core::Exception(std::move(message), "[SNT-EXS] ") {}
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-EXS] ") {}
    };

    /**
     * Conversion exceptions occur during unit conversions
     */
    class ConversionException : public exs::Exception {
      public:
        ConversionException(
            std::string message,
            std::string expected,
            std::string actual,
            std::string suggestion,
            std::string file,
            size_t line
        )
            : Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      std::move(expected),
                      std::move(actual),
                      std::move(suggestion),
                      core::SourceLocation{std::move(file), line}
                  }
              ) {};
    };

} // namespace snt::exs

#endif // SNT_EXS_EXCEPTIONS_H
