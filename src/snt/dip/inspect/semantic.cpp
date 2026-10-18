#include "value_facts.h"
#include "../nodes/metadata_fields.h"

#include <algorithm>
#include <limits>
#include <snt/core/datatypes.h>
#include <snt/core/string_format.h>
#include <snt/dip/inspect/inspector.h>
#include <stdexcept>

namespace snt::dip {
    namespace {
        std::string type_name(core::DataType type) {
            const auto found = core::DataTypeNames.find(type);
            return found == core::DataTypeNames.end() ? "unknown" : found->second;
        }
        MetadataConstraintInspection metadata_constraint(const MetadataConstraint& constraint) {
            MetadataConstraintInspection result;
            result.property = ValueMetadata::property_name(constraint.property);
            if (result.property.empty())
                result.property = "unknown";
            result.rule = constraint.rule == MetadataRule::Required ? "required" : "nonempty";
            result.schema = constraint.schema_name;
            result.origin = {constraint.source_name, constraint.source_line, {}};
            return result;
        }
    } // namespace

    SemanticDescription Inspector::describe(std::string_view path, std::size_t max_value_elements) const {
        const auto& env = *env_;
        SemanticDescription result;
        result.path = std::string(path);
        result.dependencies_recorded = this->graph().recorded;
        const auto resolved_path = env.resolve_link_path(result.path);
        if (resolved_path.followed_link) {
            result.canonical_path = (resolved_path.source_name.empty() ? "" : resolved_path.source_name + "?") +
                                    resolved_path.path;
            result.link_source = resolved_path.source_name;
            result.link_target = resolved_path.path;
            result.link_status = env.is_loaded_snapshot() && !resolved_path.source_name.empty() ? "unresolved"
                                                                                               : "resolved";
        }
        if (env.is_loaded_snapshot() && resolved_path.followed_link && !resolved_path.source_name.empty()) {
            result.kind = "link";
            result.link_status = "unresolved";
            result.value_unavailable_reason = "unresolved_remote_link";
            return result;
        }
        InspectionCapabilities capabilities;
        try {
            capabilities = this->capabilities(path);
        } catch (const dip::EnvironmentException&) {
            throw;
        }
        result.kind = capabilities.hasValue ? "value" : capabilities.hasTabularData ? "table" : "group";
        bool link_node = false;
        try {
            const auto node = env.get_node(result.path);
            if (node && node->dtype == NodeDtype::Link) {
                link_node = true;
                result.kind = "link";
                result.metadata = node->metadata;
                result.link_status = env.is_loaded_snapshot() && !node->link_source.empty() ? "unresolved"
                                                                                           : "resolved";
                result.link_source = node->link_source;
                result.link_target = node->link_target;
                const auto target = (node->link_source.empty() ? "" : node->link_source + "?") + node->link_target;
                result.canonical_path = target;
            }
        } catch (const dip::EnvironmentException&) {
        }
        if (!capabilities.hasValue && env.hierarchy.has_collection(result.path)) {
            const auto kind = env.hierarchy.get_collection(result.path).kind;
            if (!capabilities.hasTabularData && (kind == Path::Kind::Map || kind == Path::Kind::List))
                result.kind = "collection";
            result.metadata = env.hierarchy.get_collection(result.path).metadata;
        }
        if (link_node) {
            result.value_unavailable_reason = "link_node";
            return result;
        }
        if (!capabilities.hasValue) {
            result.value_unavailable_reason = "not_a_value";
            return result;
        }
        const auto facts = detail::inspect_value_facts(env, result.path, true);
        result.declared_type = type_name(facts.declared_type);
        result.stored_type = type_name(facts.stored_type);
        result.shape = facts.shape;
        result.elements = facts.elements;
        if (facts.scalar_dimension && result.elements == 1)
            result.shape.clear();
        if (max_value_elements != 0 && result.elements <= max_value_elements) {
            core::StringFormatType format;
            format.valuePrecision = std::numeric_limits<double>::max_digits10;
            auto text = facts.value->to_string(format);
            if (text.size() <= 4096)
                result.value_text = std::move(text);
            else
                result.value_unavailable_reason = "omitted_by_byte_limit";
        } else {
            result.value_unavailable_reason = "omitted_by_limit";
        }
        if (facts.units)
            result.units = facts.units->to_string();
        result.metadata = facts.metadata;
        result.tags = facts.tags;
        result.overridden = facts.overridden;
        result.declaration = facts.declaration_location;
        result.override_location = facts.override_location;
        result.source_text_available = facts.source_text_available;
        result.enforced_condition = facts.enforced_condition;
        result.enforced_options = facts.enforced_options;
        result.option_labels = facts.option_labels;
        if (result.dependencies_recorded)
            result.dependencies = this->graph().dependencies("?" + result.path);
        return result;
    }

    SemanticList Inspector::list_descriptions(
        const std::string& query, const TagFilter& tags, std::size_t limit, std::size_t max_value_elements
    ) const {
        SemanticList result;
        if (query.empty() || query.front() != '?')
            throw std::invalid_argument("Description lists require an evaluated path query beginning with ?.");
        const auto paths = select_paths(query, tags);
        result.total = paths.size();
        for (const auto& path : paths) {
            if (result.items.size() >= limit)
                break;
            result.items.push_back(describe(path, max_value_elements));
        }
        return result;
    }

    SchemaHierarchyInspection Inspector::schema_hierarchy() const {
        const auto& env = *env_;
        SchemaHierarchyInspection result;
        result.definitions_available = !env.is_loaded_snapshot();
        result.applications_complete = !env.is_loaded_snapshot();
        for (const auto& source : env.get_source_manifest())
            result.sources.emplace(source.name, source);

        if (result.definitions_available) {
            for (const auto& [name, schema] : env.schemas.entries()) {
                SchemaDefinitionInspection definition;
                definition.id = name;
                definition.name = name;
                definition.metadata = schema.metadata;
                for (const auto& constraint : schema.instance_metadata_constraints)
                    definition.instance_metadata_constraints.push_back(metadata_constraint(constraint));
                definition.origin = {schema.source_name, schema.source_line, {}};
                struct Parent {
                    size_t indent;
                    std::string path;
                    std::vector<size_t> indices;
                };
                std::vector<Parent> parents;
                for (const auto& node : schema.nodes) {
                    while (!parents.empty() && parents.back().indent >= node->indent)
                        parents.pop_back();
                    SchemaMemberInspection member;
                    member.name = node->path.name;
                    member.relative_path = (parents.empty() ? "" : parents.back().path + ".") + member.name;
                    member.origin = {node->line.source.name, node->line.source.line_number, {}};
                    member.schema_refs = node->schemas;
                    if (node->dtype == NodeDtype::Group) {
                        if (node->dtype_raw[1] == KEYWORD_MAP)
                            member.kind = "map";
                        else if (node->dtype_raw[1] == KEYWORD_LIST)
                            member.kind = "list";
                        else if (node->path.kind == Path::Kind::Map)
                            member.kind = "map_item";
                        else if (node->path.kind == Path::Kind::List)
                            member.kind = "list_item";
                        else
                            member.kind = "group";
                        if (member.schema_refs.empty())
                            member.schema_refs = node->value_raw;
                    } else if (node->dtype == NodeDtype::Table)
                        member.kind = "table";
                    else
                        member.kind = "value";
                    member.metadata = node->metadata;
                    for (const auto& constraint : node->metadata_constraints)
                        member.metadata_constraints.push_back(metadata_constraint(constraint));
                    if (const auto value = std::dynamic_pointer_cast<ValueNode>(node)) {
                        if (value->value_dtype != core::DataType::None)
                            member.type = type_name(value->value_dtype);
                        member.units = value->units_raw;
                        member.dimensions = value->dimension;
                        member.condition = value->condition;
                        for (const auto& option : value->options) {
                            member.options.push_back(
                                option.value_raw + (option.units_raw.empty() ? "" : " " + option.units_raw)
                            );
                            if (value->option_labels)
                                member.option_labels.push_back(option.label);
                        }
                    }
                    std::vector<SchemaMemberInspection>* children = &definition.members;
                    if (!parents.empty()) {
                        for (const auto index : parents.back().indices)
                            children = &children->at(index).members;
                    }
                    const size_t index = children->size();
                    children->push_back(std::move(member));
                    auto indices = parents.empty() ? std::vector<size_t>{} : parents.back().indices;
                    indices.push_back(index);
                    parents.push_back({node->indent, children->back().relative_path, std::move(indices)});
                }
                result.definitions.push_back(std::move(definition));
            }
        }

        const auto kind_for = [&](const std::string& path) {
            if (env.hierarchy.has_collection(path)) {
                switch (env.hierarchy.get_collection(path).kind) {
                case Path::Kind::Map:
                    return std::string("map");
                case Path::Kind::List:
                    return std::string("list");
                case Path::Kind::Item: {
                    const auto open = path.rfind('[');
                    const auto collection = open == std::string::npos ? "" : path.substr(0, open);
                    if (env.hierarchy.has_collection(collection))
                        return env.hierarchy.get_collection(collection).kind == Path::Kind::List
                                   ? std::string("list_item")
                                   : std::string("map_item");
                    break;
                }
                default:
                    break;
                }
            }
            return std::string("group");
        };
        if (result.applications_complete) {
            for (const auto& event : env.schema_applications()) {
                if (!result.applications.empty()) {
                    auto& previous = result.applications.back();
                    if (previous.path == event.path &&
                        previous.inherited_from_collection == event.inherited_from_collection && previous.origin &&
                        previous.origin->source == event.origin.source && previous.origin->line == event.origin.line) {
                        previous.schema_ids.push_back(event.schema_name);
                        continue;
                    }
                }
                result.applications.push_back(
                    {event.path,
                     kind_for(event.path),
                     {event.schema_name},
                     event.inherited_from_collection,
                     event.origin}
                );
            }
        } else {
            // Old snapshots retain effective associations but no declaration order or origins.
            std::vector<std::string> paths;
            for (const auto& [path, collection] : env.hierarchy.get_collections())
                if (!collection.schemas.empty())
                    paths.push_back(path);
            std::sort(paths.begin(), paths.end());
            for (const auto& path : paths) {
                const auto& collection = env.hierarchy.get_collection(path);
                result.applications.push_back({path, kind_for(path), collection.schemas, std::nullopt, std::nullopt});
            }
        }
        for (const auto& node : env.nodes.get_nodes()) {
            if (!node)
                continue;
            SchemaValueAssociation value;
            value.path = node->path.name;
            for (const auto& schema : env.get_applied_schemas(value.path))
                value.applied_schema_ids.push_back(schema.name);
            if (const auto contributor = env.get_contributing_schema(value.path))
                value.contributing_schema_id = contributor->name;
            result.values.push_back(std::move(value));
        }
        return result;
    }
} // namespace snt::dip
