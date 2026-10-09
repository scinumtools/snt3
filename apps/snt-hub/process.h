#ifndef SNT_APP_HUB_PROCESS_H
#define SNT_APP_HUB_PROCESS_H

#include <string>
#include <vector>

namespace snt::hub {

struct ProcessResult {
    int status = 0;
    std::string output;
};

ProcessResult process(const std::vector<std::string>& args, bool capture = true);

} // namespace snt::hub

#endif
