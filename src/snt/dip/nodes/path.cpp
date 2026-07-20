#include <iostream>
#include <snt/dip/nodes/path.h>
#include <snt/dip/settings.h>

namespace snt::dip {

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
                throw std::runtime_error("Name has an invalid format: " + path);
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
                        throw std::runtime_error("Node collection item has an invalid format: " + path);
                    }
                    item += c;
                    ++pos;
                }
                if (pos >= path.size() || path[pos] != ']') {
                    throw std::runtime_error("Node collection has unclosed brackets: " + path);
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
            throw std::runtime_error("Path is not fully qualified: " + path);

        if (!currentPath.empty()) {
            // A simple value or node group
            collections.push_back({std::move(currentPath), "", Path::Kind::Group});
        }

        name = path;                    // set full path name
        kind = collections.back().kind; // set final path kind
    }

} // namespace snt::dip
