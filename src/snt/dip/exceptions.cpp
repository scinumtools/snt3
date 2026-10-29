#include <snt/dip/exceptions.h>

namespace snt::dip {
    namespace {
        thread_local const Line* active_exception_line = nullptr;
    }

    const Line* current_exception_line() noexcept {
        return active_exception_line;
    }

    ExceptionLocationScope::ExceptionLocationScope(const Line& line) : previous_(active_exception_line) {
        active_exception_line = &line;
    }

    ExceptionLocationScope::~ExceptionLocationScope() {
        active_exception_line = previous_;
    }
} // namespace snt::dip
