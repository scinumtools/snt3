#ifndef SNT_EXS_EXCEPTIONS_H
#define SNT_EXS_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::exs {

    class Exception : public core::Exception {
      public:
        using core::Exception::Exception;
    };

} // namespace snt::exs

#endif // SNT_EXS_EXCEPTIONS_H
