#ifndef SNT_API_EXCEPTIONS_H
#define SNT_API_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::api {

    class Exception : public core::Exception {
      public:
        explicit Exception(std::string message) : core::Exception(std::move(message), "[SNT-API] ") {}
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-API] ") {}
    };

} // namespace snt::api

#endif // SNT_API_EXCEPTIONS_H
