#include <snt/dip/nodes/path.h>

namespace snt::dip {

    Path::Path(const std::string& path) {

        size_t pos = 0;
        std::string currentPath;

        while (pos < path.size()) {
            // parse path component
            std::string part;
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
                CollectionType type = item.empty() ? CollectionType::LIST : CollectionType::MAP;
                collections.push_back({std::move(currentPath), std::move(item), std::move(type)});
                currentPath.clear();
            }
            if (pos >= path.size() || path[pos] != '.')
                break;
            ++pos; // skip '.'
        }
        if (!currentPath.empty()) {
            // A simple or node group
            collections.push_back({std::move(currentPath), "", CollectionType::GROUP});
        }

        name = currentPath;
    }

} // namespace snt::dip
