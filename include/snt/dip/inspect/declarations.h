#ifndef SNT_DIP_DECLARATIONS_H
#define SNT_DIP_DECLARATIONS_H

#include <snt/dip/settings.h>

#include <map>
#include <optional>
#include <string>

namespace snt::dip {

/** Explicit path declarations in one evaluated source scope. Inferred parent
 * paths are intentionally absent. The first active declaration is retained.
 */
class ExplicitDeclarations {
public:
    void record(const std::string& path, const Line& line) {
        lines_.emplace(path, line);
    }

    std::optional<Line> find(const std::string& path) const {
        const auto entry = lines_.find(path);
        if (entry == lines_.end()) return std::nullopt;
        return entry->second;
    }

private:
    std::map<std::string, Line> lines_;
};

} // namespace snt::dip

#endif
