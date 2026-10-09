#ifndef SNT_APP_HUB_H
#define SNT_APP_HUB_H

#include <stdexcept>
#include <string>

namespace snt::hub {

class Error : public std::runtime_error {
    int code_;
public:
    explicit Error(std::string message, int code = 2)
        : std::runtime_error(std::move(message)), code_(code) {}
    int code() const noexcept { return code_; }
};

int command(int argc, char* argv[]);

} // namespace snt::hub

#endif
