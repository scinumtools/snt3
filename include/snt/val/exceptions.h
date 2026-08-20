#ifndef SNT_VAL_EXCEPTIONS_H
#define SNT_VAL_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::val {

    class Exception : public core::Exception {
      public:
        using core::Exception::Exception;
    };

} // namespace snt::val

#endif // SNT_VAL_EXCEPTIONS_H
