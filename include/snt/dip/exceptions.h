#ifndef SNT_DIP_EXCEPTIONS_H
#define SNT_DIP_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::dip {

    class Exception : public core::Exception {
      public:
        explicit Exception(std::string message) : core::Exception(std::move(message), "[SNT-DIP] ") {}
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-DIP] ") {}
    };

    class ParserException : public dip::Exception {
      public:
        ParserException(
            std::string message,
            std::string expected,
            std::string actual,
            std::string suggestion,
            const BaseNode* node,
            std::string file,
            size_t line
        )
            : Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      std::move(expected),
                      std::move(actual),
                      std::move(suggestion),
                      core::SourceLocation{node->line.source.name, node->line.source.line_number, node->line.code},
                      core::SourceLocation{std::move(file), line}
                  }
              ) {}
    };

    class IOException : public dip::Exception {
      public:
        IOException(
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
                      std::nullopt,
                      core::SourceLocation{std::move(file), line}
                  }
              ) {}
    };

    class SyntaxException : public dip::Exception {
      public:
        SyntaxException(
            std::string message,
            std::string expected,
            std::string actual,
            std::string suggestion,
            const BaseNode* node,
            std::string file,
            size_t line
        )
            : Exception(
                  core::ExceptionInfo{
                      std::move(message),
                      std::move(expected),
                      std::move(actual),
                      std::move(suggestion),
                      core::SourceLocation{node->line.source.name, node->line.source.line_number, node->line.code},
                      core::SourceLocation{std::move(file), line}
                  }
              ) {}
    };
} // namespace snt::dip

#endif // SNT_DIP_EXCEPTIONS_H
