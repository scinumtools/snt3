#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    Cursor::Cursor(const Environment* env, std::string_view path) : env_(env), path_(path) {
        if (path_.empty()) {
            kind = Path::Kind::Empty;
        } else {
            const Collection& col = env_->hierarchy.get_collection(path_);
            switch (col.kind) {
            case Path::Kind::None:
                throw dip::EnvironmentException(
                    "Unknown path",
                    "The requested path must correspond to a collection.",
                    "The path was not found in the collections: " + std::string(path),
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
                kind = col.kind;
                break;
            }
        }
    }

    // TODO: implement return of children Cursors for group collection

    std::vector<Cursor> Cursor::elements() const {
        std::vector<Cursor> list;
        const Collection& col = env_->hierarchy.get_collection(path_);
        if (col.kind == Path::Kind::List) {
            for (auto key : col.items) {
                list.push_back(Cursor(env_, path_ + "[" + key + "]"));
            }
        } else {
            throw dip::EnvironmentException(
                "Unknown path",
                "The requested path must correspond to a list collection: " + std::string(path_),
                "The path is the " + Path::KindNames[col.kind] + " collections.",
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
            for (auto key : col.items) {
                map.insert({key, Cursor(env_, path_ + "[" + key + "]")});
            }
        } else {
            throw dip::EnvironmentException(
                "Unknown path",
                "The requested path must correspond to a map collection: " + std::string(path_),
                "The path is the " + Path::KindNames[col.kind] + " collections.",
                "Check whether the path is correct.",
                __FILE__,
                __LINE__
            );
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
            throw dip::EnvironmentException(
                "Unknown path",
                "The requested path must correspond to a value node: " + path_,
                "The path was not found in collections.",
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
                "The requested path must correspond to a collection, or an map key.",
                "The path was not found in the collections, or items: " + new_path,
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
                "The requested index must correspond to some list item.",
                "Couldn't find items with the given index: " + new_path,
                "Check whether the path is correct.",
                __FILE__,
                __LINE__
            );
        }
        // return new cursor
        return Cursor(env_, new_path);
    }

} // namespace snt::dip
