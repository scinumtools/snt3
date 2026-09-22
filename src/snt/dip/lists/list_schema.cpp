#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_schema.h>
#include <stdexcept>
#include <utility>

namespace snt::dip {

    namespace {

        std::string next_schema_id(std::map<std::string, size_t>& counters, const std::string& parent_id) {
            const std::string scope = parent_id.empty() ? "ENV" : parent_id;
            return scope + "_SCHEMA" + std::to_string(counters[scope]++);
        }

    } // namespace

    SchemaList::SchemaList() = default;

    void SchemaList::append(const std::string& name, BaseNode::ListType& nodes, const std::string& parent_id) {
        auto it = schemas.find(name);
        if (it != schemas.end())
            throw dip::EnvironmentException(
                "Duplicate schema",
                "A schema named `" + name + "` already exists in the environment schema list.",
                "Choose a different schema name.",
                __FILE__,
                __LINE__
            );
        schemas.insert({name, {name, nodes, next_schema_id(id_counters, parent_id)}});
    }

    void SchemaList::append(const std::string& name, EnvSchema src, const std::string& parent_id) {
        auto it = schemas.find(name);
        if (it != schemas.end())
            throw dip::EnvironmentException(
                "Duplicate schema",
                "A schema named `" + name + "` already exists in the environment schema list.",
                "Choose a different schema name.",
                __FILE__,
                __LINE__
            );
        if (src.id.empty())
            src.id = next_schema_id(id_counters, parent_id);
        schemas.insert({name, std::move(src)});
    }

    EnvSchema& SchemaList::at(const std::string& name) {
        auto it = schemas.find(name);
        if (it == schemas.end())
            throw dip::EnvironmentException(
                "Unknown schema",
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
                "The schema `" + name + "` was not found in the environment schema list.",
                "Check whether the schema name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

    const std::map<std::string, EnvSchema>& SchemaList::entries() const {
        return schemas;
    }

} // namespace snt::dip
