#ifndef SNT_CORE_EXCEPTIONS_H
#define SNT_CORE_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace snt::core {

    struct ExceptionInfo {
        std::string message;
        std::string expected;
        std::string actual;
        std::string suggestion;
        std::string path;
        std::string source;
        std::size_t line = 0;
        std::size_t column = 0;
    };

    class Exception : public std::runtime_error {
      public:
        explicit Exception(std::string message) : std::runtime_error(message), m_info(ExceptionInfo({message})) {}

        explicit Exception(ExceptionInfo info) : std::runtime_error(info.message), m_info(std::move(info)) {}

        const char* what() const noexcept { return m_info.message.c_str(); }

        const ExceptionInfo& info() const noexcept { return m_info; }

      private:
        ExceptionInfo m_info;
    };

} // namespace snt::core

#endif // SNT_CORE_EXCEPTIONS_H
