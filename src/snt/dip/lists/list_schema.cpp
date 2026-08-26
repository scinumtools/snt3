#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_schema.h>
#include <stdexcept>

namespace snt::dip {

    SchemaList::SchemaList() = default;

    void SchemaList::append(const std::string& name, BaseNode::ListType& nodes) {
        auto it = schemas.find(name);
        if (it != schemas.end())
            throw dip::EnvironmentException(
                "Duplicate schema",
                "The schema name must be unique within the environment schema list.",
                "A schema named `" + name + "` already exists in the environment schema list.",
                "Choose a different schema name.",
                __FILE__,
                __LINE__
            );
        schemas.insert({name, {name, nodes}});
    }

    void SchemaList::append(const std::string& name, const EnvSchema& src) {
        auto it = schemas.find(name);
        if (it != schemas.end())
            throw dip::EnvironmentException(
                "Duplicate schema",
                "The schema name must be unique within the environment schema list.",
                "A schema named `" + name + "` already exists in the environment schema list.",
                "Choose a different schema name.",
                __FILE__,
                __LINE__
            );
        schemas.insert({name, src});
    }

    EnvSchema& SchemaList::at(const std::string& name) {
        auto it = schemas.find(name);
        if (it == schemas.end())
            throw dip::EnvironmentException(
                "Unknown schema",
                "The requested schema must exist in the environment schema list.",
                "The schema `" + name + "` was not found in the environment schema list.",
                "Check whether the schema name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

    const EnvSchema& SchemaList::at(const std::string& name) const {
        auto it = schemas.find(name);
        if (it == schemas.end())
            throw dip::EnvironmentException(
                "Unknown schema",
                "The requested schema must exist in the environment schema list.",
                "The schema `" + name + "` was not found in the environment schema list.",
                "Check whether the schema name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

} // namespace snt::dip
