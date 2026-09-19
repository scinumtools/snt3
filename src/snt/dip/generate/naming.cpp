#include "naming.h"

#include <cctype>
#include <snt/dip/exceptions.h>

namespace snt::dip::generate {
    std::string identifier(const std::string& name, std::unordered_set<std::string>& used) {
        std::string output;
        output.reserve(name.size() + 1);
        for (const unsigned char character : name) {
            if (std::isalnum(character) || character == '_')
                output += static_cast<char>(character);
            else
                output += '_';
        }
        if (output.empty() || std::isdigit(static_cast<unsigned char>(output.front())))
            output.insert(output.begin(), '_');
        if (!used.insert(output).second)
            throw dip::EnvironmentException(
                "Export name collision",
                "The DIPL name `" + name + "` maps to the duplicate native identifier `" + output + "`.",
                "Rename one of the colliding DIPL nodes before exporting.",
                __FILE__,
                __LINE__
            );
        return output;
    }
} // namespace snt::dip::generate
