#include "generate/export.h"

#include <algorithm>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/exs/exceptions.h>
#include <snt/sha256.h>
#include <unordered_set>

namespace snt::dip {

    namespace {

        SourceInfo make_source_info(const EnvSource& source) {
            return {
                source.name,
                source.path,
                source.parent.name,
                source.parent.line_number,
                "SHA-256",
                snt::sha256(source.code),
                source.metadata,
            };
        }

    } // namespace

    /**
     * Split request expression into a name and node path
     *
     * Request expects a single node {source?path}
     * Request is a root {source?path.}
     *
     * @param request Request expression
     * @return Tuple with the request source, node path and root status
     */
    inline std::tuple<std::string, std::string, bool> parse_request(const std::string& request) {
        size_t pos = request.find(SIGN_QUERY);
        if (pos == std::string::npos) {
            throw dip::EnvironmentException(
                "Invalid node request",
                "The environment request must contain a question mark; none was found in `" + request + "`.",
                "Add a question mark at the front: `?" + request + "`.",
                __FILE__,
                __LINE__
            );
        } else {
            char lastChar = request[request.size() - 1];
            if (lastChar == SIGN_SEPARATOR || lastChar == SIGN_QUERY) // request expects nodes
                return {request.substr(0, pos), request.substr(pos + 1, request.size() - pos - 2), true};
            else // request expect a single node
                return {request.substr(0, pos), request.substr(pos + 1), false};
        }
    }

    Environment::Environment() = default;

    void Environment::record_block_input(const BaseNode& declared, const BaseNode& input, const std::string& path) {
        const bool table = declared.dtype == NodeDtype::Table;
        if (table && input.value_origin == ValueOrigin::ReferenceRaw && !input.value_raw.empty())
            sources.at(input.value_raw.front()).table_text = true;
        if (!block_input_recording_ || input.value_origin != ValueOrigin::String || input.value_raw.empty())
            return;
        const bool array = (declared.dtype == NodeDtype::Boolean || declared.dtype == NodeDtype::Integer ||
                            declared.dtype == NodeDtype::Float) &&
                           !declared.dimension.empty() && input.value_shape.empty();
        if (!table && !array)
            return;
        block_inputs_.insert_or_assign(
            path,
            BlockInput{
                table ? BlockInput::Kind::Table : BlockInput::Kind::Array,
                path,
                input.value_raw.front(),
                input.line.source.name,
                input.line.source.line_number
            }
        );
    }

    Environment::DependencyScope::DependencyScope(
        Environment& env, std::string owner, DependencyEventKind kind, std::optional<core::SourceLocation> location
    )
        : env_(env), previous_(env.active_dependency_event_) {
        env_.dependency_graph_.events.push_back({std::move(owner), kind, std::move(location)});
        env_.active_dependency_event_ = env_.dependency_graph_.events.size() - 1;
    }

    Environment::DependencyScope::~DependencyScope() {
        env_.active_dependency_event_ = previous_;
    }

    exs::CompositionGraph* Environment::active_composition(const std::string& expression) {
        if (!active_dependency_event_)
            return nullptr;
        auto& event = dependency_graph_.events.at(*active_dependency_event_);
        event.expression = expression;
        event.composition.emplace();
        return &*event.composition;
    }

    void Environment::set_active_dependency_owner(std::string owner) {
        if (active_dependency_event_)
            dependency_graph_.events.at(*active_dependency_event_).owner = std::move(owner);
    }

    void Environment::set_value_controls(const std::string& owner, const std::vector<size_t>& case_ids) {
        for (auto it = dependency_graph_.events.rbegin(); it != dependency_graph_.events.rend(); ++it) {
            if (it->owner != owner || it->kind != DependencyEventKind::Value)
                continue;
            it->controlled_by.clear();
            for (const auto case_id : case_ids)
                it->controlled_by.push_back("#case:" + std::to_string(case_id));
            break;
        }
    }

    void Environment::record_import_origin(const std::string& source_node_id) {
        record_dependency(source_node_id, source_node_id);
    }

    void Environment::record_dependency(
        const std::string& target, const std::string& request, std::string_view operand
    ) const {
        if (active_dependency_event_)
            dependency_graph_.events.at(*active_dependency_event_)
                .reads.push_back({target, request, std::string(operand)});
    }

    void Environment::generate(ExportFormat format, const std::filesystem::path& file) const {
        generate::write(*this, format, file);
    }

    std::vector<SourceInfo> Environment::get_source_manifest() const {
        if (!source_manifest_.empty() || sources.entries().empty())
            return source_manifest_;

        std::vector<SourceInfo> manifest;
        manifest.reserve(sources.entries().size());
        for (const auto& entry : sources.entries())
            manifest.push_back(make_source_info(entry.second));
        return manifest;
    }

    std::optional<SourceInfo> Environment::get_source_info(const std::string& name) const {
        const auto manifest = get_source_manifest();
        const auto entry = std::find_if(manifest.begin(), manifest.end(), [&name](const SourceInfo& source) {
            return source.name == name;
        });
        if (entry == manifest.end())
            return std::nullopt;
        return *entry;
    }

    void Environment::set_source_manifest(std::vector<SourceInfo> manifest) {
        source_manifest_ = std::move(manifest);
    }

    std::vector<TraceInfo> Environment::get_trace_manifest() const {
        if (trace_manifest_loaded_)
            return trace_manifest_;

        std::vector<TraceInfo> manifest;
        manifest.reserve(units.entries().size() + schemas.entries().size() + functions.entries().size());
        for (const auto& entry : units.entries())
            manifest.push_back({entry.second.id, entry.second.name, "unit"});
        for (const auto& entry : schemas.entries())
            manifest.push_back({entry.second.id, entry.second.name, "schema"});
        for (const auto& function : functions.entries())
            manifest.push_back(
                {function.id, function.name, function.kind == FunctionKind::Value ? "function_value" : "function_nodes"}
            );
        std::sort(manifest.begin(), manifest.end(), [](const TraceInfo& lhs, const TraceInfo& rhs) {
            return lhs.id < rhs.id;
        });
        return manifest;
    }

    void Environment::set_trace_manifest(std::vector<TraceInfo> manifest) {
        trace_manifest_ = std::move(manifest);
        trace_manifest_loaded_ = true;
    }

    std::vector<SchemaInfo> Environment::get_schema_manifest() const {
        std::vector<SchemaInfo> manifest;
        if (schema_manifest_loaded_) {
            manifest = schema_manifest_;
        } else {
            manifest.reserve(schemas.entries().size());
            for (const auto& [name, schema] : schemas.entries())
                manifest.push_back({schema.id, name, schema.source_name, schema.source_line, schema.metadata, {}});
            std::sort(manifest.begin(), manifest.end(), [](const SchemaInfo& lhs, const SchemaInfo& rhs) {
                return lhs.id < rhs.id;
            });
        }
        for (auto& schema : manifest)
            schema.source = get_source_info(schema.source_name);
        return manifest;
    }

    void Environment::set_schema_manifest(std::vector<SchemaInfo> manifest) {
        schema_manifest_ = std::move(manifest);
        schema_manifest_loaded_ = true;
    }

    std::vector<SchemaInfo> Environment::get_applied_schemas(const std::string& path) const {
        const auto manifest = get_schema_manifest();
        std::vector<SchemaInfo> result;
        auto add_at = [&](const std::string& prefix, bool item_follows) {
            if (!hierarchy.has_collection(prefix))
                return;
            const auto& collection = hierarchy.get_collection(prefix);
            if ((collection.kind == Path::Kind::List || collection.kind == Path::Kind::Map) && !item_follows)
                return;
            for (const auto& name : collection.schemas) {
                const auto schema = std::find_if(manifest.begin(), manifest.end(), [&](const SchemaInfo& entry) {
                    return entry.name == name;
                });
                if (schema != manifest.end() &&
                    std::none_of(result.begin(), result.end(), [&](const SchemaInfo& entry) {
                        return entry.id == schema->id;
                    }))
                    result.push_back(*schema);
            }
        };
        for (size_t index = 0; index < path.size(); ++index)
            if (path[index] == '.' || path[index] == '[')
                add_at(path.substr(0, index), path[index] == '[');
        add_at(path, false);
        return result;
    }

    std::optional<SchemaInfo> Environment::get_contributing_schema(const std::string& path) const {
        const auto node = get_node(path);
        if (node->schema_id.empty())
            return std::nullopt;
        const auto manifest = get_schema_manifest();
        const auto schema = std::find_if(manifest.begin(), manifest.end(), [&](const SchemaInfo& entry) {
            return entry.id == node->schema_id;
        });
        if (schema == manifest.end())
            return std::nullopt;
        return *schema;
    }

    std::string Environment::request_code(const std::string& source_name) const {
        const auto& source = sources.at(source_name);
        record_dependency(source_name + "?", source_name);
        return source.code;
    }

    ValueNodeData Environment::request_node_data(
        const std::string& request, const RequestType rtype, std::string_view operand
    ) const {
        ValueNodeData new_value;

        switch (rtype) {
        case RequestType::Function: {
            FunctionList::DataFunctionType func = functions.get_value(request);
            new_value = func(*this);
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path, is_root] = parse_request(request);
            if (is_root)
                break;
            const auto resolved = resolve_link_path(node_path, source_name);
            std::vector<std::string> link_stack;
            const auto vnode = resolve_path_node(resolved.source_name, resolved.path, link_stack);
            if (vnode && vnode->dtype == NodeDtype::Link)
                throw dip::EnvironmentException(
                    "Link has no scalar value",
                    "The link at `" + vnode->path.name + "` identifies a node, not a scalar value.",
                    "Read a value below the link or inspect the link target.", __FILE__, __LINE__
                );
            if (vnode) {
                const std::string canonical = source_name + "?" + vnode->path.name;
                record_dependency(canonical, request, operand);
                new_value.value = vnode->value ? vnode->value->clone() : nullptr;
                if (vnode->units)
                    new_value.units = vnode->units;
            }
            break;
        }
        default:
            throw dip::EnvironmentException(
                "Unrecognised request type",
                "The requested type is not supported; only function and reference requests are available.",
                "Select either a function or reference request type.",
                __FILE__,
                __LINE__
            );
        }
        return new_value;
    }

    val::BaseValue::PointerType Environment::request_value(
        const std::string& request, const RequestType rtype, std::optional<std::string_view> to_unit
    ) const {
        val::BaseValue::PointerType new_value = nullptr;
        switch (rtype) {
        case RequestType::Function: {
            FunctionList::DataFunctionType func = functions.get_value(request);
            ValueNodeData new_data = func(*this);
            new_value = std::move(new_data.value);
            if (to_unit && to_unit.value() != core::KEYWORD_NONE) {
                Line line{"", Source{request, 0}};
                // NOTE: If unit conversion is not required, the to_unit should be set to
                // "none". This is usefull if we want to simply get a reference node as it is.
                if (!new_data.units && !to_unit->empty()) {
                    throw dip::UnitException(
                        "Dimension mismatch",
                        "The final quantity should have the physical dimension `" + std::string(*to_unit) +
                            "`, but the converted quantity has no physical dimensions.",
                        "Check the units of the input quantity.",
                        __FILE__,
                        __LINE__,
                        line // TODO:: this is a referenced node, we should show also referencing node
                    );
                } else if (new_data.units && to_unit->empty()) {
                    throw dip::UnitException(
                        "Dimension mismatch",
                        "The final quantity should have no physical dimensions, but the converted quantity has "
                        "physical dimensions `" +
                            new_data.units->to_string() + "`.",
                        "Check the units of the input quantity.",
                        __FILE__,
                        __LINE__,
                        line // TODO:: this is a referenced node, we should show also referencing node
                    );
                } else if (new_data.units) {
                    puq::Quantity quantity = std::move(new_value) * (*new_data.units);
                    quantity = quantity.convert(std::string(to_unit.value()));
                    new_value = std::move(quantity.measurement.result.estimate);
                    break;
                }
            }
            break;
        }
        case RequestType::Reference: {
            ValueNodeData data = request_node_data(request, rtype);
            new_value = std::move(data.value);
            if (to_unit && to_unit.value() != core::KEYWORD_NONE) {
                Line line{"", Source{request, 0}};
                if (!data.units && !to_unit->empty())
                    throw dip::UnitException(
                        "Dimension mismatch",
                        "The final quantity should have the physical dimension `" + std::string(*to_unit) +
                            "`, but the converted quantity has no physical dimensions.",
                        "Check the units of the input quantity.", __FILE__, __LINE__, line
                    );
                if (data.units && to_unit->empty())
                    throw dip::UnitException(
                        "Dimension mismatch",
                        "The final quantity should have no physical dimensions, but the converted quantity has "
                        "physical dimensions `" + data.units->to_string() + "`.",
                        "Check the units of the input quantity.", __FILE__, __LINE__, line
                    );
                if (data.units) {
                    puq::Quantity quantity = std::move(new_value) * *data.units;
                    quantity = quantity.convert(std::string(*to_unit));
                    new_value = std::move(quantity.measurement.result.estimate);
                }
            }
            break;
        }
        default:
            throw dip::EnvironmentException(
                "Unrecognised request type",
                "The requested type is not supported because only function and reference requests are available.",
                "Select either a function or reference request type.",
                __FILE__,
                __LINE__
            );
        }
        return std::move(new_value);
    }

    inline bool hasIntersection(const std::vector<std::string>& vec1, const std::vector<std::string>& vec2) {
        // Put elements of the first vector into a hash set
        std::unordered_set<std::string> set1(vec1.begin(), vec1.end());

        // Check if any element of the second vector exists in the set
        for (const auto& item : vec2) {
            if (set1.count(item)) {
                return true; // Found a match, they intersect
            }
        }
        return false; // No match found
    }

    bool TagFilter::matches(const std::vector<std::string>& tags) const {
        return std::all_of(
                   all.begin(),
                   all.end(),
                   [&tags](const std::string& tag) { return std::find(tags.begin(), tags.end(), tag) != tags.end(); }
               ) &&
               (any.empty() || hasIntersection(tags, any)) && !hasIntersection(tags, none);
    }

    std::vector<ValueNode::PointerType> Environment::selected_nodes(
        const std::string& request, const TagFilter& tags
    ) const {
        auto [source_name, node_path, is_root] = parse_request(request);
        const NodeList<ValueNode>& node_pool = source_name.empty() ? nodes : sources.at(source_name).nodes;
        std::vector<ValueNode::PointerType> selected;
        std::unordered_set<std::string> seen;
        for (const auto& vnode : node_pool.get_nodes()) {
            if (!vnode)
                continue;
            const auto& name = vnode->path.name;
            const bool descendant =
                name.size() > node_path.size() && name.rfind(node_path, 0) == 0 &&
                (name[node_path.size()] == SIGN_SEPARATOR || name[node_path.size()] == SIGN_ARRAY_OPEN);
            const bool path_matches = name == node_path || (is_root && (node_path.empty() || descendant));
            if (path_matches && tags.matches(vnode->tags) && seen.insert(name).second)
                selected.push_back(vnode);
        }
        return selected;
    }

    ValueNode::ListType Environment::request_group(
        const std::string& request, const RequestType rtype, const std::vector<std::string>& tags
    ) const {
        ValueNode::ListType new_nodes;
        switch (rtype) {
        case RequestType::Function: {
            FunctionList::NodesFunctionType func = functions.get_nodes(request);
            new_nodes = func(*this);
            for (auto& vnode : new_nodes) {
                Line line;
                line.code = "";
                line.source = Source{request + "()", 0};
                vnode->line = line;
                vnode->value_origin = ValueOrigin::FunctionRes;
            }
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path, is_root] = parse_request(request);
            std::string node_path_child = (!node_path.empty()) ? node_path + std::string(1, SIGN_SEPARATOR) : node_path;
            const NodeList<ValueNode>& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
            if (is_root) {
                // if path is a root, select its children nodes
                for (size_t i = 0; i < node_pool.size(); i++) {
                    ValueNode::PointerType vnode = node_pool.at(i);
                    if (vnode && vnode->path.name.rfind(node_path_child, 0) == 0 &&
                        vnode->path.name.size() > node_path_child.size()) {
                        // filter nodes based on tags
                        if (!tags.empty() && !hasIntersection(vnode->tags, tags))
                            continue;
                        record_dependency(source_name + "?" + vnode->path.name, request);
                        // select node
                        std::string new_name = vnode->path.name.substr(node_path_child.size(), vnode->path.name.size());
                        ValueNode::PointerType new_vnode =
                            std::dynamic_pointer_cast<ValueNode>(vnode->clone(Path(new_name), 0));
                        if (dependency_recording_)
                            new_vnode->copied_from = source_name + "?" + vnode->path.name;
                        new_nodes.push_back(new_vnode);
                    }
                }
            } else {
                // otherwise we select only a node with exactly same path
                for (size_t i = 0; i < node_pool.size(); i++) {
                    ValueNode::PointerType vnode = node_pool.at(i);
                    if (node_path == vnode->path.name) {
                        // filter nodes based on tags
                        if (!tags.empty() && !hasIntersection(vnode->tags, tags))
                            continue;
                        record_dependency(source_name + "?" + vnode->path.name, request);
                        // select node
                        std::string new_name = vnode->path.basename();
                        ValueNode::PointerType new_vnode =
                            std::dynamic_pointer_cast<ValueNode>(vnode->clone(Path(new_name), 0));
                        if (dependency_recording_)
                            new_vnode->copied_from = source_name + "?" + vnode->path.name;
                        new_nodes.push_back(new_vnode);
                        break;
                    }
                }
            }
            break;
        }
        default:
            throw dip::EnvironmentException(
                "Unrecognised request type",
                "The requested type is not supported because only function and reference requests are available.",
                "Select either a function or reference request type.",
                __FILE__,
                __LINE__
            );
        }
        if (new_nodes.empty())
            throw dip::EnvironmentException(
                "Node request returns empty node group",
                "The request `" + request + "` did not match any nodes.",
                "Check whether the request is correct.",
                __FILE__,
                __LINE__
            );
        return new_nodes;
    }

    std::unordered_map<std::string, ValueNode::ListType> Environment::request_map(
        const std::string& request, const RequestType rtype, const std::vector<std::string>& tags
    ) const {
        std::unordered_map<std::string, ValueNode::ListType> map;
        switch (rtype) {
        case RequestType::Function: {
            throw dip::MissingException("Request map from functions is not implemented yet.", __FILE__, __LINE__);
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path, is_root] = parse_request(request);
            std::string node_path_child = (!node_path.empty()) ? node_path + std::string(1, SIGN_SEPARATOR) : node_path;
            const NodeList<ValueNode>& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
            const HierarchyList& hlist =
                hierarchy; //(source_name.empty()) ? hierarchy : sources.at(source_name).hierarchy;
            if (!source_name.empty())
                throw dip::MissingException("Request map from sources is not implemented yet.", __FILE__, __LINE__);

            const Collection& col = hlist.get_collection(node_path);
            if (col.kind == Path::Kind::List)
                throw dip::EnvironmentException(
                    "Requested collection has incorrect type",
                    "The requested collection must be a map, but `" + request + "` refers to a list.",
                    "Check whether the requested path is correct.",
                    __FILE__,
                    __LINE__
                );
            for (const auto& item : col.items) {
                std::string child_request = request;
                child_request += '[';
                child_request += item;
                child_request += ']';
                child_request += SIGN_SEPARATOR;
                map.insert({item, request_group(child_request)});
            }
            break;
        }
        default:
            throw dip::EnvironmentException(
                "Unrecognised request type",
                "The requested type is not supported because only function and reference requests are available.",
                "Select either a function or reference request type.",
                __FILE__,
                __LINE__
            );
        }
        if (map.empty())
            throw dip::EnvironmentException(
                "Node request returns empty node map",
                "The request `" + request + "` did not match any nodes.",
                "Check whether the request is correct.",
                __FILE__,
                __LINE__
            );
        return map;
    }

    std::vector<ValueNode::ListType> Environment::request_list(
        const std::string& request, const RequestType rtype, const std::vector<std::string>& tags
    ) const {
        std::vector<ValueNode::ListType> list;
        switch (rtype) {
        case RequestType::Function: {
            throw dip::MissingException("Request list from functions is not implemented yet.", __FILE__, __LINE__);
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path, is_root] = parse_request(request);
            std::string node_path_child = (!node_path.empty()) ? node_path + std::string(1, SIGN_SEPARATOR) : node_path;
            const NodeList<ValueNode>& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
            const HierarchyList& hlist =
                hierarchy; //(source_name.empty()) ? hierarchy : sources.at(source_name).hierarchy;
            if (!source_name.empty())
                throw dip::MissingException("Request list from sources is not implemented yet.", __FILE__, __LINE__);

            const Collection& col = hlist.get_collection(node_path);
            if (col.kind == Path::Kind::Map)
                throw dip::EnvironmentException(
                    "Requested collection has incorrect type",
                    "The requested collection must be a list, but `" + request + "` refers to a map.",
                    "Check whether the requested path is correct.",
                    __FILE__,
                    __LINE__
                );

            for (const auto& item : col.items) {
                std::string child_request = request;
                child_request += '[';
                child_request += item;
                child_request += ']';
                child_request += SIGN_SEPARATOR;
                list.push_back(request_group(child_request));
            }
            break;
        }
        default:
            throw dip::EnvironmentException(
                "Unrecognised request type",
                "The requested type is not supported because only function and reference requests are available.",
                "Select either a function or reference request type.",
                __FILE__,
                __LINE__
            );
        }
        if (list.empty())
            throw dip::EnvironmentException(
                "Node request returns empty node list",
                "The request `" + request + "` did not match any nodes.",
                "Check whether the request is correct.",
                __FILE__,
                __LINE__
            );
        return list;
    }

    val::BaseValue::PointerType Environment::get_value(size_t index) const {
        const auto node = nodes.at(index);
        return node->value ? node->value->clone() : nullptr;
    }

    ValueNode::PointerType Environment::get_node(const std::string& path, const std::string& source_name) const {
        if (source_name.empty())
            for (const auto& node : nodes.get_nodes())
                if (node && node->path.name == path && node->dtype == NodeDtype::Link)
                    return node;
        std::vector<std::string> link_stack;
        const auto resolved = resolve_link_path(path, source_name);
        if (auto node = resolve_path_node(resolved.source_name, resolved.path, link_stack))
            return node;
        throw dip::EnvironmentException(
            "Path does not exist",
            "The requested path `" + path + "` does not point to any node.",
            "Check whether the path is correct.",
            __FILE__,
            __LINE__
        );
    }

    ResolvedPath Environment::resolve_link_path(const std::string& path, const std::string& source_name) const {
        ResolvedPath result{source_name, path, false};
        std::unordered_set<std::string> visited;
        while (true) {
            const NodeList<ValueNode>* pool = &nodes;
            if (!result.source_name.empty()) {
                const auto source = sources.entries().find(result.source_name);
                if (snapshot_loaded_ || source == sources.entries().end())
                    return result;
                pool = &source->second.nodes;
            }
            ValueNode::PointerType link;
            for (const auto& candidate : pool->get_nodes()) {
                if (!candidate || candidate->dtype != NodeDtype::Link)
                    continue;
                const std::string& alias = candidate->path.name;
                if (result.path == alias ||
                    (result.path.size() > alias.size() && result.path.compare(0, alias.size(), alias) == 0 &&
                     (result.path[alias.size()] == SIGN_SEPARATOR || result.path[alias.size()] == SIGN_ARRAY_OPEN))) {
                    link = candidate;
                    break;
                }
            }
            if (!link)
                return result;
            const std::string identity = result.source_name + "?" + link->path.name;
            if (!visited.insert(identity).second)
                throw dip::EnvironmentException(
                    "Link cycle", "Following `" + identity + "` creates a link cycle.",
                    "Change one of the links so that its target chain terminates.", __FILE__, __LINE__, link->line
                );
            const std::string suffix = result.path.substr(link->path.name.size());
            result.source_name = link->link_source;
            result.path = link->link_target + suffix;
            result.followed_link = true;
        }
    }

    void Environment::validate_write_path(const std::string& path) const {
        const auto resolved = resolve_link_path(path);
        if (!resolved.followed_link)
            return;
        const std::string canonical = resolved.source_name.empty() ? resolved.path :
                                                                     resolved.source_name + "?" + resolved.path;
        throw dip::SyntaxException(
            "Read-only link",
            "The path `" + path + "` traverses a read-only link to `" + canonical + "`.",
            "Address modifications and overrides to the canonical target path.", __FILE__, __LINE__
        );
    }

    ValueNode::PointerType Environment::resolve_path_node(
        const std::string& source_name, const std::string& path, std::vector<std::string>& link_stack
    ) const {
        const NodeList<ValueNode>* pool = &nodes;
        if (!source_name.empty()) {
            if (snapshot_loaded_)
                throw dip::EnvironmentException(
                    "Unresolved remote link",
                    "The target `" + source_name + "?" + path + "` is remote and is not loaded in this DIPH5 snapshot.",
                    "Load the original DIPL project with the remote source available.", __FILE__, __LINE__
                );
            const auto source = sources.entries().find(source_name);
            if (source == sources.entries().end())
                throw dip::EnvironmentException(
                    "Unknown source",
                    "The source `" + source_name + "` was not found in the environment source list.",
                    "Check whether the source name is correct.", __FILE__, __LINE__
                );
            pool = &source->second.nodes;
        }
        for (const auto& node : pool->get_nodes()) {
            if (node && node->path.name == path)
                return node;
        }
        for (const auto& node : pool->get_nodes()) {
            if (!node || node->dtype != NodeDtype::Link)
                continue;
            const std::string& alias = node->path.name;
            if (path.size() <= alias.size() || path.compare(0, alias.size(), alias) != 0 ||
                (path[alias.size()] != SIGN_SEPARATOR && path[alias.size()] != SIGN_ARRAY_OPEN))
                continue;
            const std::string identity = (source_name.empty() ? "" : source_name + "?") + alias;
            if (std::find(link_stack.begin(), link_stack.end(), identity) != link_stack.end())
                throw dip::EnvironmentException(
                    "Link cycle", "Following `" + identity + "` creates a link cycle.",
                    "Change one of the links so that its target chain terminates.", __FILE__, __LINE__, node->line
                );
            link_stack.push_back(identity);
            const std::string suffix = path.substr(alias.size());
            auto resolved = resolve_path_node(node->link_source, node->link_target + suffix, link_stack);
            link_stack.pop_back();
            return resolved;
        }
        return nullptr;
    }

    void Environment::validate_links() const {
        const auto validate_pool = [&](const NodeList<ValueNode>& pool, const std::string& owner) {
            for (const auto& node : pool.get_nodes()) {
                if (!node || node->dtype != NodeDtype::Link)
                    continue;
                const std::string child_prefix = node->path.name + SIGN_SEPARATOR;
                const std::string item_prefix = node->path.name + SIGN_ARRAY_OPEN;
                for (const auto& candidate : pool.get_nodes())
                    if (candidate && candidate.get() != node.get() &&
                        (candidate->path.name.rfind(child_prefix, 0) == 0 ||
                         candidate->path.name.rfind(item_prefix, 0) == 0))
                        throw dip::EnvironmentException(
                            "Link has local children",
                            "The link `" + owner + node->path.name + "` also declares local descendant `" +
                                candidate->path.name + "`.",
                            "Declare descendants at the canonical target path instead.", __FILE__, __LINE__, candidate->line
                        );
                std::unordered_set<std::string> visited;
                auto current = node;
                ValueNode::PointerType target;
                while (current) {
                    const std::string identity = current->link_source + "?" + current->link_target;
                    if (!visited.insert(identity).second)
                        throw dip::EnvironmentException(
                            "Link cycle", "The link `" + owner + node->path.name + "` participates in a target cycle.",
                            "Change one of the links so that its target chain terminates.", __FILE__, __LINE__, node->line
                        );
                    std::vector<std::string> stack;
                    target = resolve_path_node(current->link_source, current->link_target, stack);
                    if (!target || target->dtype != NodeDtype::Link)
                        break;
                    current = target;
                }
                bool exists = static_cast<bool>(target);
                if (!exists && node->link_source.empty())
                    exists = hierarchy.has_collection(node->link_target);
                if (!exists && !node->link_source.empty()) {
                    const auto source = sources.entries().find(node->link_source);
                    exists = source != sources.entries().end() &&
                             source->second.hierarchy.has_collection(node->link_target);
                }
                if (!exists)
                    throw dip::EnvironmentException(
                        "Unresolved link target",
                        "The link `" + owner + node->path.name + "` targets missing node `" +
                            (node->link_source.empty() ? "?" : node->link_source + "?") + node->link_target + "`.",
                        "Correct the target path or define the target node.", __FILE__, __LINE__, node->line
                    );
            }
        };
        validate_pool(nodes, "");
    }

    const std::string Environment::to_string() const {
        return "Environment";
    }

    Cursor Environment::operator[](std::string_view path) const {
        return Cursor(this, path);
    }

} // namespace snt::dip
