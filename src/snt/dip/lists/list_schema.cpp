#include <snt/dip/lists/list_schema.h>
#include <stdexcept>

namespace snt::dip {

    SchemaList::SchemaList() = default;

    void SchemaList::append(const std::string& name, BaseNode::ListType& nodes) {
        auto it = schemas.find(name);
        if (it != schemas.end())
            throw std::invalid_argument("Schema with the same name already exists: " + name);
        schemas.insert({name, {name, nodes}});
    }

    void SchemaList::append(const std::string& name, const EnvSchema& src) {
        auto it = schemas.find(name);
        if (it != schemas.end())
            throw std::invalid_argument("Schema with the same name already exists: " + name);
        schemas.insert({name, src});
    }

    EnvSchema& SchemaList::at(const std::string& name) {
        auto it = schemas.find(name);
        if (it == schemas.end())
            throw std::runtime_error("Following schema was not found in the environment schema list: " + name);
        else
            return it->second;
    }

    const EnvSchema& SchemaList::at(const std::string& name) const {
        auto it = schemas.find(name);
        if (it == schemas.end())
            throw std::runtime_error("Following schema was not found in the environment schema list: " + name);
        else
            return it->second;
    }

} // namespace snt::dip
