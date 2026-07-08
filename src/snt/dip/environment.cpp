#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_value.h>
#include <unordered_set>

namespace snt::dip {

    /**
     * @brief Split request expression into a name and node path
     *
     * @param request Request expression
     * @return Tuple with the request name and node path
     */
    inline std::tuple<std::string, std::string> parse_request(const std::string& request) {
        size_t pos = request.find(SIGN_QUERY);
        if (pos == std::string::npos)
            throw std::runtime_error("Environment request must contain a question mark symbol: " + request);
        else
            return {request.substr(0, pos), request.substr(pos + 1)};
    }

    Environment::Environment() = default;

    std::string Environment::request_code(const std::string& source_name) const {
        return sources.at(source_name).code;
    }

    ValueNodeData Environment::request_node_data(const std::string& request, const RequestType rtype) const {
        ValueNodeData new_value;

        switch (rtype) {
        case RequestType::Function: {
            // TODO: this needs to be implemented
            std::runtime_error("Functions in the request_node_data are not implemented yet.");
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path] = parse_request(request);
            const NodeList<ValueNode>& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
            for (size_t i = 0; i < node_pool.size(); i++) {
                ValueNode::PointerType vnode = node_pool.at(i);
                if (vnode && vnode->path.name == node_path) {
                    new_value.value = vnode->value->clone();
                    if (vnode->units)
                        new_value.units = vnode->units;
                    break;
                }
            }
            break;
        }
        default:
            throw std::runtime_error("Unrecognized environment request type");
        }
        // NOTE: We need to enable this in order to implemente the def/ndef logical operators
        // if (new_value.value == nullptr)
        //    throw std::runtime_error("Value node data environment request returns an empty
        //    pointer: " + request);
        return new_value;
    }

    val::BaseValue::PointerType Environment::request_value(
        const std::string& request, const RequestType rtype, const std::string& to_unit
    ) const {
        val::BaseValue::PointerType new_value = nullptr;
        bool found = false;
        switch (rtype) {
        case RequestType::Function: {
            FunctionList::ValueFunctionType func = functions.get_value(request);
            new_value = func(*this);
            found = true;
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path] = parse_request(request);
            const NodeList<ValueNode>& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
            for (size_t i = 0; i < node_pool.size(); i++) {
                ValueNode::PointerType vnode = node_pool.at(i);
                if (vnode && vnode->path.name == node_path) {
                    found = true;
                    if (vnode->value != nullptr) {
                        new_value = vnode->value->clone();
                    } else {
                        new_value = nullptr;
                        break;
                    }
                    if (to_unit != core::KEYWORD_NONE) {
                        // NOTE: If unit conversion is not required, the to_unit should be set to
                        // "none".
                        //       This is usefull if we want to simply get a reference node as it is.
                        if (!vnode->units && !to_unit.empty()) {
                            throw std::runtime_error(
                                "Request: Trying to convert nondimensional quantity into '" + to_unit +
                                "': " + vnode->line.code
                            );
                        } else if (vnode->units && to_unit.empty()) {
                            throw std::runtime_error(
                                "Request: Trying to convert '" + vnode->units_raw +
                                "' into a nondimensional quantity: " + vnode->line.code
                            );
                        } else if (vnode->units) {
                            puq::Quantity quantity = std::move(new_value) * (*vnode->units);
                            quantity = quantity.convert(to_unit);
                            new_value = std::move(quantity.measurement.result.estimate);
                            break;
                        }
                    }
                }
            }
            break;
        }
        default:
            throw std::runtime_error("Unrecognized environment request type");
        }
        if (!found)
            throw std::runtime_error("Value environment request returns an empty pointer: " + request);
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

    ValueNode::ListType Environment::request_group(
        const std::string& request, const RequestType rtype, const std::vector<std::string>& tags
    ) const {
        ValueNode::ListType new_nodes;
        switch (rtype) {
        case RequestType::Function: {
            FunctionList::TableFunctionType func = functions.get_table(request);
            new_nodes = func(*this);
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path] = parse_request(request);
            std::string node_path_child = (!node_path.empty()) ? node_path + std::string(1, SIGN_SEPARATOR) : node_path;
            const NodeList<ValueNode>& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
            // if node has children, select them all
            size_t p = node_pool.size(); // parent node index
            for (size_t i = 0; i < node_pool.size(); i++) {
                ValueNode::PointerType vnode = node_pool.at(i);
                if (vnode && vnode->path.name.rfind(node_path_child, 0) == 0 &&
                    vnode->path.name.size() > node_path_child.size()) {
                    // filter nodes based on tags
                    if (!tags.empty() && !hasIntersection(vnode->tags, tags))
                        continue;
                    // select node
                    std::string new_name = vnode->path.name.substr(node_path_child.size(), vnode->path.name.size());
                    new_nodes.push_back(vnode->clone(Path(new_name)));
                } else if (vnode && vnode->path.name == node_path) {
                    p = i;
                }
            }
            // if node does not have childs, but exists, return it
            if (new_nodes.empty() && p < node_pool.size()) {
                ValueNode::PointerType vnode = node_pool.at(p);
                size_t pos = node_path.find_last_of('.');
                if (pos != std::string::npos) {
                    std::string new_name = vnode->path.name.substr(pos + 1, node_path.size());
                    new_nodes.push_back(vnode->clone(Path(new_name)));
                } else {
                    new_nodes.push_back(vnode->clone(Path(node_path)));
                }
            }
            break;
        }
        default:
            throw std::runtime_error("Unrecognized environment request type");
        }
        if (new_nodes.empty())
            throw std::runtime_error("Node environment request returns an empty node group: " + request);
        return new_nodes;
    }

    std::unordered_map<std::string, ValueNode::ListType> Environment::request_map(
        const std::string& request, const RequestType rtype, const std::vector<std::string>& tags
    ) const {
        std::unordered_map<std::string, ValueNode::ListType> map;
        switch (rtype) {
        case RequestType::Function: {
            // TODO: implement functions
            throw std::runtime_error("Request map from functions is not implemented yet.");
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path] = parse_request(request);
            std::string node_path_child = (!node_path.empty()) ? node_path + std::string(1, SIGN_SEPARATOR) : node_path;
            const NodeList<ValueNode>& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
            const HierarchyList& hlist =
                hierarchy;            //(source_name.empty()) ? hierarchy : sources.at(source_name).hierarchy;
            if (!source_name.empty()) // TODO: implemente request map from source
                throw std::runtime_error("Request map from sources is not implemented yet.");

            const Collection& col = hlist.get_collection(node_path);
            if (col.type == Path::CollectionType::LIST)
                throw std::runtime_error("Requested collection is a list: " + request);

            for (const auto& item : col.items) {
                map.insert({item, request_group(request + "[" + item + "]")});
            }
            break;
        }
        default:
            throw std::runtime_error("Unrecognized environment request type");
        }
        if (map.empty())
            throw std::runtime_error("Node environment request returns an empty node map: " + request);
        return map;
    }

    std::vector<ValueNode::ListType> Environment::request_list(
        const std::string& request, const RequestType rtype, const std::vector<std::string>& tags
    ) const {
        std::vector<ValueNode::ListType> list;
        switch (rtype) {
        case RequestType::Function: {
            // TODO: implement functions
            throw std::runtime_error("Request list from functions is not implemented yet.");
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path] = parse_request(request);
            std::string node_path_child = (!node_path.empty()) ? node_path + std::string(1, SIGN_SEPARATOR) : node_path;
            const NodeList<ValueNode>& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
            const HierarchyList& hlist =
                hierarchy;            //(source_name.empty()) ? hierarchy : sources.at(source_name).hierarchy;
            if (!source_name.empty()) // TODO: implemente request list from source
                throw std::runtime_error("Request list from sources is not implemented yet.");

            const Collection& col = hlist.get_collection(node_path);
            if (col.type == Path::CollectionType::MAP)
                throw std::runtime_error("Requested collection is a map: " + request);

            for (const auto& item : col.items) {
                list.push_back(request_group(request + "[" + item + "]"));
            }
            break;
        }
        default:
            throw std::runtime_error("Unrecognized environment request type");
        }
        if (list.empty())
            throw std::runtime_error("Node environment request returns an empty node list: " + request);
        return list;
    }

    Cursor Environment::request_cursor() {
        return Cursor(this);
    }

    val::BaseValue::PointerType Environment::get_value(size_t index) const {
        return nodes.at(index)->value->clone();
    }

} // namespace snt::dip
