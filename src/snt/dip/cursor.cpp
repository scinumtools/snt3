#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    Cursor::Cursor(const Environment* env, std::string_view path) : env_(env), path_(path) {
        if (!path_.empty()) {
            const Collection& col = env_->hierarchy.get_collection(path_);
            switch (col.kind) {
            case Path::Kind::NONE:
                throw std::runtime_error(
                    "Requested path does not point to any kind of hierarchy: " + std::string(path)
                );
                break;
            case Path::Kind::VALUE:
            case Path::Kind::GROUP:
            case Path::Kind::LIST:
            case Path::Kind::MAP:
            case Path::Kind::ITEM:
                kind = col.kind;
                break;
            }
        }
    }

    Cursor Cursor::get_group(std::string_view path) const {
        std::string new_path = path_.empty() ? std::string(path) : path_ + "." + std::string(path);
        ValueNode::ListType nodes = env_->request_group("?" + new_path);
        if (!nodes.empty())
            return Cursor(env_, new_path);
        else
            throw std::runtime_error("Requested path does not point to any group: " + new_path);
    }

    std::vector<Cursor> Cursor::get_list(std::string_view path) const {
        std::string new_path = path_.empty() ? std::string(path) : path_ + "." + std::string(path);
        std::vector<Cursor> list;
        const Collection& col = env_->hierarchy.get_collection(new_path);
        if (col.kind == Path::Kind::LIST) {
            for (auto key : col.items) {
                list.push_back(Cursor(env_, new_path + "[" + key + "]"));
            }
        } else {
            throw std::runtime_error("Requested path does not point to a list: " + new_path);
        }
        return list;
    }

    std::unordered_map<std::string, Cursor> Cursor::get_map(std::string_view path) const {
        std::string new_path = path_.empty() ? std::string(path) : path_ + "." + std::string(path);
        std::unordered_map<std::string, Cursor> map;
        const Collection& col = env_->hierarchy.get_collection(new_path);
        if (col.kind == Path::Kind::MAP) {
            for (auto key : col.items) {
                map.insert({key, Cursor(env_, new_path + "[" + key + "]")});
            }
        } else {
            throw std::runtime_error("Requested path does not point to a map: " + new_path);
        }
        return map;
    }

    const std::string& Cursor::get_path() const {
        return path_;
    }

    val::Array::ShapeType Cursor::get_shape() const {
        val::BaseValue::PointerType value = env_->request_value("?" + path_);
        if (value)
            return value->get_shape();
        else
            throw std::runtime_error("Requested path does not point to a value node: " + path_);
    }

    const std::string Cursor::to_string() const {
        val::BaseValue::PointerType value = env_->request_value("?" + path_);
        if (value)
            return "Cursor('" + path_ + "', " + value->to_string() + ")";
        else
            return "Cursor('" + path_ + "')";
    }

} // namespace snt::dip
