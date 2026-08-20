#ifndef SNT_MAT_EXCEPTIONS_H
#define SNT_MAT_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::mat {

    class Exception : public core::Exception {
      public:
        using core::Exception::Exception;
    };

} // namespace snt::mat

#endif // SNT_MAT_EXCEPTIONS_H
