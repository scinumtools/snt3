#ifndef SNT_DIP_GENERATE_NAMING_H
#define SNT_DIP_GENERATE_NAMING_H

#include <string>
#include <unordered_set>

namespace snt::dip::generate {
    /** Convert a DIPL name into a portable native-language identifier. */
    std::string identifier(const std::string& name, std::unordered_set<std::string>& used);
}

#endif // SNT_DIP_GENERATE_NAMING_H
