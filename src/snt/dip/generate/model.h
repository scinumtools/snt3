#ifndef SNT_DIP_GENERATE_MODEL_H
#define SNT_DIP_GENERATE_MODEL_H

#include <snt/core/datatypes.h>

#include <memory>
#include <string>
#include <vector>

namespace snt::dip {
    class Environment;
}

namespace snt::dip::generate {

    enum class NodeKind {
        Root,
        Group,
        Map,
        MapItem,
        List,
        ListItem,
        Value,
    };

    struct Node {
        NodeKind kind = NodeKind::Group;
        std::string name;
        std::string path;
        core::DataType dtype = core::DataType::None;
        std::string literal;
        std::vector<size_t> shape;
        bool is_array = false;
        bool has_value = false;
        std::vector<std::unique_ptr<Node>> children;
    };

    /** Build a language-neutral tree from evaluated DIPL values and collections. */
    std::unique_ptr<Node> build_model(const Environment& environment);

} // namespace snt::dip::generate

#endif // SNT_DIP_GENERATE_MODEL_H
