#include <algorithm>
#include <iostream>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/path.h>
#include <snt/dip/settings.h>
#include <stdexcept>

namespace snt::dip {

    std::unordered_map<Path::Kind, std::string> Path::KindNames{
        {Kind::None, "none"},
        {Kind::Empty, "empty"},
        {Kind::Group, "group"},
        {Kind::Map, "map"},
        {Kind::List, "list"},
        {Kind::Item, "item"},
    };

    Path::Path(const std::string& path) {
        if (path.empty()) {
            // If path is empty the node is not a value, nor a group.
            // This can happen if property, source, unit, or similar nodes are being cloned.
            name = "";               // set full path name
            kind = Path::Kind::None; // set final path kind
            return;
        }

        size_t pos = 0;
        std::string currentPath;

        while (pos < path.size()) {
            // parse path component
            std::string part;
            if (path[pos] == SIGN_CONDITION) {
                part += path[pos];
                ++pos;
            }
            while (pos < path.size()) {
                char c = path[pos];
                if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-') {
                    part += c;
                    ++pos;
                } else {
                    break;
                }
            }
            if (part.empty()) {
                throw dip::SyntaxException(
                    "Invalid name",
                    "The name contains characters that are not allowed: `" + path + "`.",
                    "Use only letters, digits, underscores, and hyphens in the name.",
                    __FILE__,
                    __LINE__
                );
            }
            if (!currentPath.empty())
                currentPath += '.';
            currentPath += part;
            // collection?
            if (pos < path.size() && path[pos] == '[') {
                ++pos;
                std::string item;
                while (pos < path.size() && path[pos] != ']') {
                    char c = path[pos];
                    if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '*')) {
                        throw dip::SyntaxException(
                            "Invalid node collection item",
                            "The node collection item contains an invalid character `" + std::string(1, c) +
                                "` in path: `" + path + "`",
                            "Use only letters, digits, underscores, hyphens, and `*` in the node collection item.",
                            __FILE__,
                            __LINE__
                        );
                    }
                    item += c;
                    ++pos;
                }
                if (pos >= path.size() || path[pos] != ']') {
                    throw dip::SyntaxException(
                        "Unclosed brackets",
                        "The node collection is missing its closing bracket `]`: `" + path + "`",
                        "Add a closing `]` bracket to complete the node collection.",
                        __FILE__,
                        __LINE__
                    );
                }
                ++pos; // skip ']'
                Path::Kind type = item.empty() ? Path::Kind::List : Path::Kind::Map;
                collections.push_back({currentPath, std::move(item), std::move(type)});
                currentPath.clear();
            }
            if (pos >= path.size() || path[pos] != '.')
                break;
            ++pos; // skip '.'
        }
        if (pos != path.size())
            if (pos != path.size()) {
                throw dip::SyntaxException(
                    "Path is not fully qualified",
                    "The path contains unexpected characters after the complete path expression: `" + path + "`.",
                    "Remove the unexpected characters and ensure that path nodes names contain only letters, digits, "
                    "underscores, and hyphens.",
                    __FILE__,
                    __LINE__
                );
            }

        if (!currentPath.empty()) {
            // A simple value or node group
            collections.push_back({std::move(currentPath), "", Path::Kind::Group});
        }

        name = path;                    // set full path name
        kind = collections.back().kind; // set final path kind
    }

    Path Path::resolve(const std::string& path) {
        if (path == ".")
            return name;
        const std::size_t nDots = path.find_first_not_of('.');
        if (nDots == 0)
            return Path(path);
        // Number of components in the current path.
        const std::size_t nComponents = 1 + std::count(name.begin(), name.end(), '.');
        if (nDots > nComponents)
            throw dip::SyntaxException(
                "Relative path exceeds root",
                "The relative path attempts to access a parent beyond the root node: `" + path + "`.",
                "Reduce the number of parent references `.` so that the path does not go beyond the root node.",
                __FILE__,
                __LINE__
            );
        std::string result = name;
        for (std::size_t i = 0; i < nDots; ++i) {
            const std::size_t pos = result.rfind('.');
            if (pos == std::string::npos) {
                // Removing the root component.
                result.clear();
            } else {
                result.erase(pos);
            }
        }
        const std::string relative = path.substr(nDots);
        if (!relative.empty()) {
            if (!result.empty())
                result += '.';
            result += relative;
        }
        return Path(result);
    }

} // namespace snt::dip
