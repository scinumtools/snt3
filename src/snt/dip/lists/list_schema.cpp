#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_schema.h>
#include <functional>
#include <set>
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

    void SchemaList::append(
        const std::string& name,
        BaseNode::ListType& nodes,
        const std::string& parent_id,
        const ValueMetadata& metadata,
        size_t source_line,
        const std::vector<MetadataConstraint>& instance_constraints
    ) {
        auto it = schemas.find(name);
        if (it != schemas.end())
            throw dip::EnvironmentException(
                "Duplicate schema",
                "A schema named `" + name + "` already exists in the environment schema list.",
                "Choose a different schema name.",
                __FILE__,
                __LINE__
            );
        EnvSchema schema{name, nodes, next_schema_id(id_counters, parent_id), metadata, instance_constraints, parent_id, source_line};
        schemas.insert({name, std::move(schema)});
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

    void SchemaList::import_referenced_from(const SchemaList& source, const std::vector<std::string>& names) {
        std::set<std::string> visited;
        std::vector<std::string> ordered;
        std::function<void(const std::string&)> visit = [&](const std::string& name) {
            if (!visited.insert(name).second)
                return;
            const EnvSchema& schema = source.at(name);
            // Schema members can themselves apply schemas to groups or collections.
            for (const auto& node : schema.nodes) {
                if (!node || node->dtype != NodeDtype::Group)
                    continue;
                for (const auto& dependency : node->value_raw)
                    visit(dependency);
                for (const auto& dependency : node->schemas)
                    visit(dependency);
            }
            ordered.push_back(name);
        };
        for (const auto& name : names)
            visit(name);

        // Check every name before changing the destination registry. Repeated
        // imports of the same source may reuse a definition; unrelated ones may not.
        for (const auto& name : ordered) {
            const auto existing = schemas.find(name);
            if (existing == schemas.end())
                continue;
            const EnvSchema& incoming = source.at(name);
            if (existing->second.id != incoming.id || existing->second.source_name != incoming.source_name ||
                existing->second.source_line != incoming.source_line)
                throw dip::EnvironmentException(
                    "Conflicting imported schema",
                    "The imported schema `" + name + "` conflicts with a definition already registered in this environment.",
                    "Rename one schema or import sources with distinct schema names.",
                    __FILE__, __LINE__
                );
        }
        for (const auto& name : ordered)
            if (schemas.find(name) == schemas.end())
                append(name, source.at(name));
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
