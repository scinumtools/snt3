#ifndef SNT_DIP_EXCEPTIONS_H
#define SNT_DIP_EXCEPTIONS_H

#include <snt/core/exceptions.h>

namespace snt::dip {

    class Exception : public core::Exception {
      public:
        using core::Exception::Exception;
    };

} // namespace snt::dip

#endif // SNT_DIP_EXCEPTIONS_H
