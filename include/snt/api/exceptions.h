#ifndef SNT_API_EXCEPTIONS_H
#define SNT_API_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::api {

    class Exception : public core::Exception {
      public:
        using core::Exception::Exception;
    };

} // namespace snt::api

#endif // SNT_API_EXCEPTIONS_H
