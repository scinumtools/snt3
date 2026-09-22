#include "generate/export.h"

#include <array>
#include <cstdint>
#include <iomanip>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/exs/exceptions.h>
#include <sstream>
#include <unordered_set>

namespace snt::dip {

    namespace {

        constexpr std::array<uint32_t, 64> SHA256_ROUND_CONSTANTS = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
        };

        constexpr uint32_t rotate_right(uint32_t value, uint32_t amount) {
            return (value >> amount) | (value << (32 - amount));
        }

        std::string sha256(const std::string& input) {
            std::vector<uint8_t> bytes;
            bytes.reserve(input.size() + 72);
            for (unsigned char character : input)
                bytes.push_back(character);

            const uint64_t bit_length = static_cast<uint64_t>(bytes.size()) * 8;
            bytes.push_back(0x80);
            while (bytes.size() % 64 != 56)
                bytes.push_back(0);
            for (int shift = 56; shift >= 0; shift -= 8)
                bytes.push_back(static_cast<uint8_t>(bit_length >> shift));

            std::array<uint32_t, 8> hash = {
                0x6a09e667,
                0xbb67ae85,
                0x3c6ef372,
                0xa54ff53a,
                0x510e527f,
                0x9b05688c,
                0x1f83d9ab,
                0x5be0cd19,
            };

            for (size_t block = 0; block < bytes.size(); block += 64) {
                std::array<uint32_t, 64> words{};
                for (size_t index = 0; index < 16; ++index) {
                    const size_t offset = block + index * 4;
                    words[index] = (static_cast<uint32_t>(bytes[offset]) << 24) |
                                   (static_cast<uint32_t>(bytes[offset + 1]) << 16) |
                                   (static_cast<uint32_t>(bytes[offset + 2]) << 8) |
                                   static_cast<uint32_t>(bytes[offset + 3]);
                }
                for (size_t index = 16; index < words.size(); ++index) {
                    const uint32_t s0 = rotate_right(words[index - 15], 7) ^ rotate_right(words[index - 15], 18) ^
                                        (words[index - 15] >> 3);
                    const uint32_t s1 = rotate_right(words[index - 2], 17) ^ rotate_right(words[index - 2], 19) ^
                                        (words[index - 2] >> 10);
                    words[index] = words[index - 16] + s0 + words[index - 7] + s1;
                }

                uint32_t a = hash[0];
                uint32_t b = hash[1];
                uint32_t c = hash[2];
                uint32_t d = hash[3];
                uint32_t e = hash[4];
                uint32_t f = hash[5];
                uint32_t g = hash[6];
                uint32_t h = hash[7];
                for (size_t index = 0; index < words.size(); ++index) {
                    const uint32_t s1 = rotate_right(e, 6) ^ rotate_right(e, 11) ^ rotate_right(e, 25);
                    const uint32_t choice = (e & f) ^ ((~e) & g);
                    const uint32_t temp1 = h + s1 + choice + SHA256_ROUND_CONSTANTS[index] + words[index];
                    const uint32_t s0 = rotate_right(a, 2) ^ rotate_right(a, 13) ^ rotate_right(a, 22);
                    const uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
                    const uint32_t temp2 = s0 + majority;
                    h = g;
                    g = f;
                    f = e;
                    e = d + temp1;
                    d = c;
                    c = b;
                    b = a;
                    a = temp1 + temp2;
                }
                hash[0] += a;
                hash[1] += b;
                hash[2] += c;
                hash[3] += d;
                hash[4] += e;
                hash[5] += f;
                hash[6] += g;
                hash[7] += h;
            }

            std::ostringstream output;
            output << std::hex << std::setfill('0');
            for (uint32_t value : hash)
                output << std::setw(8) << value;
            return output.str();
        }

        SourceInfo make_source_info(const EnvSource& source) {
            return {
                source.name,
                source.path,
                source.parent.name,
                source.parent.line_number,
                "SHA-256",
                sha256(source.code),
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

    std::string Environment::request_code(const std::string& source_name) const {
        return sources.at(source_name).code;
    }

    ValueNodeData Environment::request_node_data(const std::string& request, const RequestType rtype) const {
        ValueNodeData new_value;

        switch (rtype) {
        case RequestType::Function: {
            FunctionList::DataFunctionType func = functions.get_value(request);
            new_value = func(*this);
            break;
        }
        case RequestType::Reference: {
            auto [source_name, node_path, is_root] = parse_request(request);
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
            auto [source_name, node_path, is_root] = parse_request(request);
            const NodeList<ValueNode>& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
            for (size_t i = 0; i < node_pool.size(); i++) {
                ValueNode::PointerType vnode = node_pool.at(i);
                if (vnode && vnode->path.name == node_path) {
                    if (vnode->value != nullptr) {
                        new_value = vnode->value->clone();
                    } else {
                        new_value = nullptr;
                        break;
                    }
                    if (to_unit && to_unit.value() != core::KEYWORD_NONE) {
                        // NOTE: If unit conversion is not required, the to_unit should be set to
                        // "none". This is usefull if we want to simply get a reference node as it is.
                        if (!vnode->units && !to_unit->empty()) {
                            throw dip::UnitException(
                                "Dimension mismatch",
                                "The final quantity should have the physical dimension `" + std::string(*to_unit) +
                                    "`, but the converted quantity has no physical dimensions.",
                                "Check the units of the input quantity.",
                                __FILE__,
                                __LINE__,
                                vnode->line // TODO:: this is a referenced node, we should show also referencing node
                            );
                        } else if (vnode->units && to_unit->empty()) {
                            throw dip::UnitException(
                                "Dimension mismatch",
                                "The final quantity should have no physical dimensions, but the converted quantity has "
                                "physical dimensions `" +
                                    vnode->units_raw + "`.",
                                "Check the units of the input quantity.",
                                __FILE__,
                                __LINE__,
                                vnode->line // TODO:: this is a referenced node, we should show also referencing node
                            );
                        } else if (vnode->units) {
                            puq::Quantity quantity = std::move(new_value) * (*vnode->units);
                            quantity = quantity.convert(std::string(to_unit.value()));
                            new_value = std::move(quantity.measurement.result.estimate);
                            break;
                        }
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
                        // select node
                        std::string new_name = vnode->path.name.substr(node_path_child.size(), vnode->path.name.size());
                        ValueNode::PointerType new_vnode =
                            std::dynamic_pointer_cast<ValueNode>(vnode->clone(Path(new_name), 0));
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
                        // select node
                        std::string new_name = vnode->path.basename();
                        ValueNode::PointerType new_vnode =
                            std::dynamic_pointer_cast<ValueNode>(vnode->clone(Path(new_name), 0));
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
        return nodes.at(index)->value->clone();
    }

    ValueNode::PointerType Environment::get_node(const std::string& path) const {

        for (const auto& node : nodes.get_nodes()) {
            if (node->path.name == path)
                return node;
        }
        throw dip::EnvironmentException(
            "Path does not exist",
            "The requested path `" + path + "` does not point to any node.",
            "Check whether the path is correct.",
            __FILE__,
            __LINE__
        );
    }

    const std::string Environment::to_string() const {
        return "Environment";
    }

    Cursor Environment::operator[](std::string_view path) const {
        return Cursor(this, path);
    }

} // namespace snt::dip
