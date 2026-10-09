#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    namespace {
        bool has_group_descendants(const HierarchyList& hierarchy, const std::string& path) {
            const std::string prefix = path + ".";
            for (const auto& [candidate, collection] : hierarchy.get_collections()) {
                (void)collection;
                if (candidate.size() > prefix.size() && candidate.compare(0, prefix.size(), prefix) == 0)
                    return true;
            }
            return false;
        }
    } // namespace

    Cursor::Cursor(const Environment* env, std::string_view path) : env_(env), path_(path) {
        if (path_.empty()) {
            kind = Path::Kind::Empty;
        } else if (!env_->hierarchy.has_collection(path_) && has_group_descendants(env_->hierarchy, path_)) {
            kind = Path::Kind::Group;
        } else {
            const Collection& col = env_->hierarchy.get_collection(path_);
            switch (col.kind) {
            case Path::Kind::None:
                throw dip::EnvironmentException(
                    "Unknown path",
                    "The path `" + std::string(path) + "` was not found in the collections.",
                    "Check whether the path is correct.",
                    __FILE__,
                    __LINE__
                );
                break;
            case Path::Kind::Empty:
            case Path::Kind::Group:
            case Path::Kind::List:
            case Path::Kind::Map:
            case Path::Kind::Item:
            case Path::Kind::Root:
                kind = col.kind;
                break;
            }
        }
    }

    std::unordered_map<std::string, Cursor> Cursor::children() const {
        if (kind != Path::Kind::Empty && kind != Path::Kind::Group && kind != Path::Kind::Item)
            throw dip::EnvironmentException(
                "Wrong collection kind",
                "The path `" + path_ + "` must correspond to a group or collection item, but it refers to a " +
                    Path::KindNames.at(kind) + " collection.",
                "Use children() on a group or item; use elements() for a list or items() for a map.",
                __FILE__,
                __LINE__
            );

        std::unordered_map<std::string, Cursor> children;
        const std::string prefix = path_.empty() ? "" : path_ + ".";
        for (const auto& [candidate, collection] : env_->hierarchy.get_collections()) {
            (void)collection;
            if (candidate.size() <= prefix.size() || candidate.compare(0, prefix.size(), prefix) != 0)
                continue;
            const std::string remainder = candidate.substr(prefix.size());
            const std::size_t end = remainder.find_first_of(".[");
            const std::string name = remainder.substr(0, end);
            if (!name.empty() && children.find(name) == children.end())
                children.emplace(name, Cursor(env_, prefix + name));
        }
        return children;
    }

    std::vector<Cursor> Cursor::elements() const {
        std::vector<Cursor> list;
        const Collection& col = env_->hierarchy.get_collection(path_);
        if (col.kind == Path::Kind::List) {
            for (const auto& key : col.items) {
                list.push_back(Cursor(env_, path_ + "[" + key + "]"));
            }
        } else {
            throw dip::EnvironmentException(
                "Wrong collection kind",
                "The path `" + std::string(path_) + "` must correspond to a list collection, but it refers to " +
                    Path::KindNames[col.kind] + " collection.",
                "Check whether the path is correct.",
                __FILE__,
                __LINE__
            );
        }
        return list;
    }

    std::unordered_map<std::string, Cursor> Cursor::items() const {
        std::unordered_map<std::string, Cursor> map;
        const Collection& col = env_->hierarchy.get_collection(path_);
        if (col.kind == Path::Kind::Map) {
            for (const auto& key : col.items) {
                map.insert({key, Cursor(env_, path_ + "[" + key + "]")});
            }
        } else {
            throw dip::EnvironmentException(
                "Wrong collection kind",
                "The path `" + std::string(path_) + "` must correspond to a map collection, but it refers to a " +
                    Path::KindNames[col.kind] + " collection.",
                "Check whether the path is correct.",
                __FILE__,
                __LINE__
            );
        }
        return map;
    }

    bool Cursor::has_item(const std::string& item) const {
        const Collection& col = env_->hierarchy.get_collection(path_);
        if (col.kind == Path::Kind::Map) {
            for (const auto& key : col.items) {
                if (key == item)
                    return true;
            }
            return false;
        } else {
            throw dip::EnvironmentException(
                "Wrong collection kind",
                "The path `" + std::string(path_) + "` must correspond to a map collection, but it refers to a " +
                    Path::KindNames[col.kind] + " collection.",
                "Check whether the path is correct.",
                __FILE__,
                __LINE__
            );
        }
    }

    const std::string& Cursor::get_path() const {
        return path_;
    }

    val::BaseValue::PointerType Cursor::get_value() const {
        const auto node = env_->get_node(path_);
        return node->value ? node->value->clone() : nullptr;
    }

    std::optional<puq::Quantity> Cursor::get_units() const {
        return env_->get_node(path_)->units;
    }

    dip::ValueNode::PointerType Cursor::get_node() const {
        return env_->get_node(path_);
    }

    Provenance Cursor::get_provenance() const {
        const dip::ValueNode::PointerType node = get_node();
        Provenance provenance{
            node->line.source.name,
            node->line.source.line_number,
            node->line.code,
            node->metadata,
            env_->get_source_info(node->line.source.name),
        };
        if (node->override) {
            provenance.override_source = env_->get_source_info(node->override_line.source.name);
            provenance.override_line = node->override_line.source.line_number;
            provenance.override_code = node->override_line.code;
        }
        return provenance;
    }

    val::Array::ShapeType Cursor::get_shape() const {
        val::BaseValue::PointerType value = env_->request_value("?" + path_);
        if (value)
            return value->get_shape();
        else
            throw dip::EnvironmentException(
                "Unknown path",
                "The path `" + path_ + "` must correspond to a value node, but it was not found in the collections.",
                "Check whether the path is correct.",
                __FILE__,
                __LINE__
            );
    }

    const Path::Kind Cursor::get_kind() const {
        return kind;
    }

    const std::string Cursor::to_string() const {
        val::BaseValue::PointerType value = env_->request_value("?" + path_);
        if (value)
            return "Cursor('" + path_ + "', " + value->to_string() + ")";
        else
            return "Cursor('" + path_ + "')";
    }

    Cursor Cursor::operator[](std::string_view path) const {
        // determine new path
        std::string new_path = path_;
        switch (kind) {
        case Path::Kind::Map:
            new_path += "[" + std::string(path) + "]";
            break;
        case Path::Kind::Item:
        case Path::Kind::Group:
            new_path += "." + std::string(path);
            break;
        case Path::Kind::Empty:
            new_path += std::string(path);
            break;
        default:
            throw dip::EnvironmentException(
                "Unknown path",
                "The path `" + new_path +
                    "` must correspond to a collection or map key, but it was not found in the collections or items.",
                "Check whether the path is correct.",
                __FILE__,
                __LINE__
            );
            break;
        }
        // return new cursor
        return Cursor(env_, new_path);
    }

    Cursor Cursor::operator[](size_t index) const {
        // determine new path
        std::string new_path = path_;
        switch (kind) {
        case Path::Kind::List:
            new_path += "[" + std::to_string(index) + "]";
            break;
        default:
            throw dip::EnvironmentException(
                "Unknown path",
                "The index `" + new_path + "` does not correspond to a list item.",
                "Check whether the list index is correct.",
                __FILE__,
                __LINE__
            );
        }
        // return new cursor
        return Cursor(env_, new_path);
    }

} // namespace snt::dip
