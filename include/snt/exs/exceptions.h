#ifndef SNT_EXS_EXCEPTIONS_H
#define SNT_EXS_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::exs {

    class Exception : public core::Exception {
      public:
        explicit Exception(std::string message) : core::Exception(std::move(message), "[SNT-EXS] ") {}
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-EXS] ") {}
    };

} // namespace snt::exs

#endif // SNT_EXS_EXCEPTIONS_H
