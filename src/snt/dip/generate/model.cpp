#include "model.h"

#include <snt/dip/environment.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace snt::dip::generate {
    namespace {
        struct Segment {
            std::string name;
            std::string item;
        };

        std::vector<Segment> split_path(const std::string& path) {
            std::vector<Segment> output;
            size_t begin = 0;
            while (begin < path.size()) {
                const size_t end = path.find('.', begin);
                const std::string part = path.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
                const size_t bracket = part.find('[');
                if (bracket == std::string::npos)
                    output.push_back({part, {}});
                else
                    output.push_back({part.substr(0, bracket), part.substr(bracket + 1, part.size() - bracket - 2)});
                if (end == std::string::npos)
                    break;
                begin = end + 1;
            }
            return output;
        }

        Node* child(Node& parent, NodeKind kind, const std::string& name, const std::string& path) {
            const auto found = std::find_if(parent.children.begin(), parent.children.end(), [&](const auto& node) {
                return node->kind == kind && node->name == name;
            });
            if (found != parent.children.end())
                return found->get();
            parent.children.push_back(std::make_unique<Node>(Node{kind, name, path}));
            return parent.children.back().get();
        }

        const Collection* collection_at(
            const std::unordered_map<std::string, Collection>& collections, const std::string& path
        ) {
            const auto found = collections.find(path);
            return found == collections.end() ? nullptr : &found->second;
        }
    } // namespace

    std::unique_ptr<Node> build_model(const Environment& environment) {
        auto root = std::make_unique<Node>();
        root->kind = NodeKind::Root;
        const auto& collections = environment.hierarchy.get_collections();

        for (const auto& value_node : environment.nodes.get_nodes()) {
            if (!value_node || !value_node->value)
                continue;

            Node* current = root.get();
            std::string raw_path;
            const auto segments = split_path(value_node->path.name);
            for (size_t index = 0; index < segments.size(); ++index) {
                const Segment& segment = segments[index];
                if (!raw_path.empty())
                    raw_path += '.';
                raw_path += segment.name;

                if (segment.item.empty()) {
                    current = child(*current, NodeKind::Group, segment.name, raw_path);
                    continue;
                }

                const Collection* collection = collection_at(collections, raw_path);
                const NodeKind collection_kind = collection && collection->kind == Path::Kind::List ? NodeKind::List : NodeKind::Map;
                current = child(*current, collection_kind, segment.name, raw_path);
                raw_path += '[' + segment.item + ']';
                current = child(
                    *current,
                    collection_kind == NodeKind::List ? NodeKind::ListItem : NodeKind::MapItem,
                    segment.item,
                    raw_path
                );
            }

            current->kind = NodeKind::Value;
            current->dtype = value_node->value->get_dtype();
            current->shape = value_node->value->get_shape();
            core::StringFormatType format;
            format.valuePrecision = std::numeric_limits<double>::max_digits10;
            current->literal = value_node->value->to_string(format);
        }
        return root;
    }

} // namespace snt::dip::generate
