#ifndef SNT_DIP_DIAGNOSTIC_H
#define SNT_DIP_DIAGNOSTIC_H

#include <exception>
#include <snt/core/diagnostic.h>

namespace snt::dip {

    /** Convert an existing DIP or core exception without parsing its formatted what() text.
     * The code identifies the exception category; individual messages do not yet have codes.
     */
    core::Diagnostic diagnostic_from_exception(const std::exception& error);

} // namespace snt::dip

#endif
