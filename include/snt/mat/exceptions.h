#ifndef SNT_MAT_EXCEPTIONS_H
#define SNT_MAT_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::mat {

    class Exception : public core::Exception {
      public:
        explicit Exception(std::string message) : core::Exception(std::move(message), "[SNT-MAT] ") {}
        explicit Exception(core::ExceptionInfo info) : core::Exception(std::move(info), "[SNT-MAT] ") {}
    };

} // namespace snt::mat

#endif // SNT_MAT_EXCEPTIONS_H
