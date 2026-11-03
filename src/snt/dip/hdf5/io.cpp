#include "io.h"
#include "io_private.h"

#include "../nodes/metadata_fields.h"
#include "../input/utf8.h"
#include "schema.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <hdf5.h>
#include <limits>
#include <map>
#include "../../../../external/nlohmann/json.hpp"
#include <optional>
#include <random>
#include <set>
#include <snt/dip/environment.h>
#include <snt/dip/experiment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/inspect/inspector.h>
#include <snt/sha256.h>
#include <snt/val/values.h>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_set>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace snt::dip::hdf5 {
    using namespace detail;
    namespace {
        using namespace schema;

        class PersistedValueNode final : public ValueNode {
          private:
            val::BaseValue::PointerType cast_scalar_value(const std::string&) const override { return nullptr; }
            val::BaseValue::PointerType cast_array_value(
                const val::Array::StringType&, const val::Array::ShapeType&
            ) const override {
                return nullptr;
            }

          public:
            PersistedValueNode(
                const Path& path,
                val::BaseValue::PointerType value,
                NodeDtype node_type,
                std::optional<puq::Quantity> units
            )
                : ValueNode(path, std::move(value), node_type, std::move(units)) {}

            PersistedValueNode(const Path& path, core::DataType value_type, NodeDtype node_type)
                : ValueNode(path, value_type, node_type) {}

            BaseNode::PointerType clone(const Path& path, std::optional<size_t> indent = std::nullopt) const override {
                auto copy = std::make_shared<PersistedValueNode>(*this);
                copy->path = path;
                if (indent)
                    copy->indent = *indent;
                return copy;
            }

            std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override {
                if (!value)
                    return std::string(core::KEYWORD_NONE);
                std::string output = value->to_string(format);
                if (units)
                    output += " " + units->to_string();
                return output;
            }
        };

        std::string data_type_name(core::DataType type) {
            auto it = core::DataTypeNames.find(type);
            if (it == core::DataTypeNames.end())
                throw dip::IOException(
                    "Unsupported value type",
                    "The value data type cannot be stored in HDF5.",
                    "Use a concrete DIPL scalar or array type.",
                    __FILE__,
                    __LINE__
                );
            return it->second;
        }

        core::DataType data_type(const std::string& name) {
            for (const auto& [type, type_name] : core::DataTypeNames)
                if (type_name == name)
                    return type;
            throw dip::IOException(
                "Unsupported value type",
                "The HDF5 value type `" + name + "` is not supported.",
                "Use a file written with a compatible SciNumTools3 version.",
                __FILE__,
                __LINE__
            );
        }

        std::string node_type_name(NodeDtype type) {
            auto it = NodeDtypeNames.find(type);
            return it == NodeDtypeNames.end() ? "none" : it->second;
        }

        NodeDtype node_type(const std::string& name) {
            for (const auto& [type, type_name] : NodeDtypeNames)
                if (type_name == name)
                    return type;
            throw Error("Unsupported DIPH5 node type: " + name);
        }

        std::string path_kind_name(Path::Kind kind) {
            switch (kind) {
            case Path::Kind::Group:
                return "group";
            case Path::Kind::Map:
                return "map";
            case Path::Kind::List:
                return "list";
            case Path::Kind::Item:
                return "item";
            default:
                return "group";
            }
        }

        // These sections share the small HDF5 helpers above; keep their boundaries explicit.
        Id ensure_group(hid_t file, const std::string& path, const std::string& kind) {
            const htri_t exists = H5Lexists(file, path.c_str(), H5P_DEFAULT);
            if (exists < 0)
                throw Error("Unable to inspect an HDF5 group");
            Id group(
                exists > 0 ? H5Gopen2(file, path.c_str(), H5P_DEFAULT)
                           : H5Gcreate2(file, path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                H5Gclose,
                "Unable to open or create an HDF5 group"
            );
            // A value group is also an intermediate group for its descendants.
            // Do not let a later child write turn it back into a plain group.
            if (!(exists > 0 && kind == "group" && read_string(group, ATTR_KIND) == KIND_VALUE_GROUP))
                write_string(group, ATTR_KIND, kind);
            return group;
        }

        std::string hdf5_path(const std::string& path) {
            std::string result = "/";
            for (char character : path) {
                if (character == '.')
                    result.push_back('/');
                else if (character == '[')
                    result.push_back('/');
                else if (character != ']')
                    result.push_back(character);
            }
            return result;
        }

        void write_order(hid_t file, const std::string& path, uint64_t order) {
            Id object(H5Oopen(file, path.c_str(), H5P_DEFAULT), H5Oclose, "Unable to open an ordered DIPH5 node");
            write_scalar<uint64_t>(object, ATTR_ORDER, H5T_NATIVE_UINT64, order);
        }

        void write_hierarchy_structure(hid_t file, const Environment& env) {
            std::unordered_set<std::string> value_paths;
            std::unordered_set<std::string> paths_with_descendants;
            for (const auto& node : env.nodes.get_nodes()) {
                if (!node)
                    continue;
                const std::string& path = node->path.name;
                value_paths.insert(path);
                size_t separator = path.find_first_of(".[");
                while (separator != std::string::npos) {
                    paths_with_descendants.insert(path.substr(0, separator));
                    separator = path.find_first_of(".[", separator + 1);
                }
            }
            std::vector<const Collection*> collections;
            for (const auto& [path, collection] : env.hierarchy.get_collections())
                collections.push_back(&collection);
            std::sort(collections.begin(), collections.end(), [](const auto* left, const auto* right) {
                return left->path.size() < right->path.size();
            });
            for (const Collection* collection : collections) {
                const bool leaf_value = value_paths.find(collection->path) != value_paths.end() &&
                                        paths_with_descendants.find(collection->path) == paths_with_descendants.end();
                if (leaf_value)
                    continue;
                std::string kind;
                switch (collection->kind) {
                case Path::Kind::Group:
                    kind = "group";
                    break;
                case Path::Kind::Map:
                    kind = "map";
                    break;
                case Path::Kind::List:
                    kind = "list";
                    break;
                case Path::Kind::Item: {
                    const size_t open = collection->path.rfind('[');
                    if (open == std::string::npos)
                        continue;
                    const auto parent = env.hierarchy.get_collection(collection->path.substr(0, open));
                    kind = parent.kind == Path::Kind::List ? "list_item" : "map_item";
                    break;
                }
                default:
                    continue;
                }
                Id group = ensure_group(file, hdf5_path(collection->path), kind);
                write_string(group, ATTR_PATH, collection->path);
                write_strings(group, ATTR_SCHEMAS, collection->schemas);
            }

            const auto write_children = [&](const std::string& parent, const std::vector<std::string>& children) {
                for (size_t index = 0; index < children.size(); ++index) {
                    const std::string child = parent.empty() ? children[index] : parent + "." + children[index];
                    const std::string path = hdf5_path(child);
                    const htri_t exists = H5Lexists(file, path.c_str(), H5P_DEFAULT);
                    if (exists <= 0)
                        throw Error("Unable to find hierarchy child `" + child + "` while saving DIPH5 order");
                    write_order(file, path, static_cast<uint64_t>(index));
                }
            };
            write_children({}, env.hierarchy.root_children_order());
            for (const auto* collection : collections) {
                if (collection->kind == Path::Kind::Group || collection->kind == Path::Kind::Item)
                    write_children(collection->path, collection->children);
                else if (collection->kind == Path::Kind::Map) {
                    for (size_t index = 0; index < collection->items.size(); ++index) {
                        const std::string child = collection->path + "[" + collection->items[index] + "]";
                        const std::string path = hdf5_path(child);
                        const htri_t exists = H5Lexists(file, path.c_str(), H5P_DEFAULT);
                        if (exists <= 0)
                            throw Error("Unable to find map item `" + child + "` while saving DIPH5 order");
                        write_order(file, path, static_cast<uint64_t>(index));
                    }
                }
            }
        }

        std::string dataset_path(hid_t file, const Environment& env, const std::string& path) {
            std::string hdf5_path;
            std::string dipl_path;
            size_t pos = 0;
            while (pos < path.size()) {
                const size_t start = pos;
                while (pos < path.size() && path[pos] != '.' && path[pos] != '[')
                    ++pos;
                const std::string name = path.substr(start, pos - start);
                if (!dipl_path.empty())
                    dipl_path += '.';
                dipl_path += name;
                hdf5_path += '/' + name;

                bool collection_item = false;
                if (pos < path.size() && path[pos] == '[') {
                    const size_t end = path.find(']', pos);
                    const std::string item = path.substr(pos + 1, end - pos - 1);
                    const Collection& collection = env.hierarchy.get_collection(dipl_path);
                    Id container = ensure_group(file, hdf5_path, path_kind_name(collection.kind));
                    write_string(container, ATTR_PATH, dipl_path);
                    if (!has_attribute(container, ATTR_SCHEMAS))
                        write_strings(container, ATTR_SCHEMAS, collection.schemas);
                    hdf5_path += '/' + item;
                    dipl_path += '[' + item + ']';
                    Id item_group =
                        ensure_group(file, hdf5_path, collection.kind == Path::Kind::List ? "list_item" : "map_item");
                    write_string(item_group, ATTR_PATH, dipl_path);
                    if (env.hierarchy.has_collection(dipl_path) && !has_attribute(item_group, ATTR_SCHEMAS))
                        write_strings(item_group, ATTR_SCHEMAS, env.hierarchy.get_collection(dipl_path).schemas);
                    if (collection.kind == Path::Kind::List)
                        write_scalar<uint64_t>(item_group, ATTR_INDEX, H5T_NATIVE_UINT64, std::stoull(item));
                    else
                        write_string(item_group, ATTR_KEY, item);
                    pos = end + 1;
                    collection_item = true;
                }

                if (pos == path.size())
                    return hdf5_path;
                if (path[pos] == '.') {
                    if (!collection_item) {
                        Id group = ensure_group(file, hdf5_path, "group");
                        write_string(group, ATTR_PATH, dipl_path);
                        if (env.hierarchy.has_collection(dipl_path) && !has_attribute(group, ATTR_SCHEMAS))
                            write_strings(group, ATTR_SCHEMAS, env.hierarchy.get_collection(dipl_path).schemas);
                    }
                    ++pos;
                }
            }
            return hdf5_path;
        }

        Id value_space(const ValueNode& node) {
            if (!node.value)
                return Id(H5Screate(H5S_NULL), H5Sclose, "Unable to create an HDF5 null dataspace");
            if (node.dimension.empty())
                return Id(H5Screate(H5S_SCALAR), H5Sclose, "Unable to create an HDF5 scalar dataspace");
            std::vector<hsize_t> dimensions;
            for (size_t dimension : node.value->get_shape())
                dimensions.push_back(dimension);
            return Id(
                H5Screate_simple(static_cast<int>(dimensions.size()), dimensions.data(), nullptr),
                H5Sclose,
                "Unable to create an HDF5 array dataspace"
            );
        }

        template <typename Stored, typename Disk>
        void write_numeric(hid_t file, const std::string& path, const ValueNode& node, hid_t type) {
            const auto* value = dynamic_cast<const val::ArrayValue<Stored>*>(node.value.get());
            if (!value)
                throw dip::IOException(
                    "Invalid value storage",
                    "The value at `" + node.path.name + "` has inconsistent storage.",
                    "Check the value data type.",
                    __FILE__,
                    __LINE__
                );
            const std::vector<Stored> stored = value->get_values();
            for (const auto item : stored) {
                if constexpr (std::is_integral_v<Disk> && std::is_integral_v<Stored> && sizeof(Disk) < sizeof(Stored)) {
                    if (item < static_cast<Stored>(std::numeric_limits<Disk>::lowest()) ||
                        item > static_cast<Stored>(std::numeric_limits<Disk>::max()))
                        throw Error("DIPH5 value exceeds its declared integer dtype");
                }
                if constexpr (std::is_floating_point_v<Disk> && std::is_floating_point_v<Stored> &&
                              sizeof(Disk) < sizeof(Stored)) {
                    if (std::isfinite(item) && std::abs(item) > std::numeric_limits<Disk>::max())
                        throw Error("DIPH5 value exceeds its declared floating-point dtype");
                }
                if (node.value_dtype == core::DataType::Boolean && item != 0 && item != 1)
                    throw Error("Invalid DIPH5 boolean value");
            }
            std::vector<Disk> data(stored.begin(), stored.end());
            Id space = value_space(node);
            Id dataset(
                H5Dcreate2(file, path.c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                H5Dclose,
                "Unable to create an HDF5 dataset"
            );
            check(
                H5Dwrite(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data()), "Unable to write an HDF5 dataset"
            );
        }

        void write_value(hid_t file, const std::string& path, const ValueNode& node) {
            if (!node.value) {
                Id space = value_space(node);
                Id dataset(
                    H5Dcreate2(file, path.c_str(), H5T_NATIVE_UINT8, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Dclose,
                    "Unable to create an HDF5 null dataset"
                );
                return;
            }
            switch (node.value_dtype) {
            case core::DataType::Boolean:
                write_numeric<uint8_t, uint8_t>(file, path, node, H5T_NATIVE_UINT8);
                break;
            case core::DataType::Character:
                write_numeric<int8_t, int8_t>(file, path, node, H5T_NATIVE_INT8);
                break;
            case core::DataType::Integer8:
                write_numeric<int64_t, int8_t>(file, path, node, H5T_NATIVE_INT8);
                break;
            case core::DataType::Integer16:
                write_numeric<int64_t, int16_t>(file, path, node, H5T_NATIVE_INT16);
                break;
            case core::DataType::Integer32:
                write_numeric<int64_t, int32_t>(file, path, node, H5T_NATIVE_INT32);
                break;
            case core::DataType::Integer64:
                write_numeric<int64_t, int64_t>(file, path, node, H5T_NATIVE_INT64);
                break;
            case core::DataType::Integer8_U:
                write_numeric<uint64_t, uint8_t>(file, path, node, H5T_NATIVE_UINT8);
                break;
            case core::DataType::Integer16_U:
                write_numeric<uint64_t, uint16_t>(file, path, node, H5T_NATIVE_UINT16);
                break;
            case core::DataType::Integer32_U:
                write_numeric<uint64_t, uint32_t>(file, path, node, H5T_NATIVE_UINT32);
                break;
            case core::DataType::Integer64_U:
                write_numeric<uint64_t, uint64_t>(file, path, node, H5T_NATIVE_UINT64);
                break;
            case core::DataType::Float32:
                write_numeric<double, float>(file, path, node, H5T_NATIVE_FLOAT);
                break;
            case core::DataType::Float64:
                write_numeric<double, double>(file, path, node, H5T_NATIVE_DOUBLE);
                break;
            case core::DataType::Float128:
                write_numeric<long double, long double>(file, path, node, H5T_NATIVE_LDOUBLE);
                break;
            case core::DataType::String: {
                const auto* value = dynamic_cast<const val::ArrayValue<std::string>*>(node.value.get());
                if (!value) throw Error("Invalid DIPH5 string value storage");
                const auto values = value->get_values();
                size_t width = 1;
                for (const auto& item : values) {
                    if (!valid_utf8(item)) throw Error("Invalid UTF-8 in DIPH5 string value");
                    width = std::max(width, item.size() + 1);
                }
                Id type(H5Tcopy(H5T_C_S1), H5Tclose, "Unable to create an HDF5 string type");
                check(H5Tset_size(type, width), "Unable to size an HDF5 string type");
                check(H5Tset_cset(type, H5T_CSET_UTF8), "Unable to set HDF5 UTF-8 encoding");
                check(H5Tset_strpad(type, H5T_STR_NULLTERM), "Unable to set HDF5 string padding");
                std::vector<char> buffer(checked_product(values.size(), width, "string dataset size"), '\0');
                for (size_t i = 0; i < values.size(); ++i)
                    std::copy(values[i].begin(), values[i].end(), buffer.begin() + static_cast<ptrdiff_t>(i * width));
                Id space = value_space(node);
                Id dataset(
                    H5Dcreate2(file, path.c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Dclose,
                    "Unable to create an HDF5 string dataset"
                );
                check(
                    H5Dwrite(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()),
                    "Unable to write an HDF5 string dataset"
                );
                break;
            }
            default:
                throw dip::IOException(
                    "Unsupported value type",
                    "The value at `" + node.path.name + "` cannot be stored in HDF5.",
                    "Use a supported DIPL value type.",
                    __FILE__,
                    __LINE__
                );
            }
        }

        void write_metadata(hid_t object, const ValueMetadata& metadata) {
#define SNT_WRITE_METADATA(TYPE, FIELD, NAME)                                                                          \
    if (!metadata.FIELD.empty())                                                                                       \
        write_string(object, NAME, metadata.FIELD);
            SNT_DIP_METADATA_SCALAR_FIELDS(SNT_WRITE_METADATA)
#undef SNT_WRITE_METADATA
#define SNT_WRITE_METADATA_LIST(TYPE, FIELD, NAME)                                                                     \
    if (!metadata.FIELD.empty())                                                                                       \
        write_strings(object, NAME, metadata.FIELD);
            SNT_DIP_METADATA_LIST_FIELDS(SNT_WRITE_METADATA_LIST)
#undef SNT_WRITE_METADATA_LIST
            write_strings(object, "metadata_present", metadata.present_names());
        }

        void write_value_dataset(hid_t file, const std::string& path, const ValueNode& node) {
            write_value(file, path, node);
            Id dataset(H5Dopen2(file, path.c_str(), H5P_DEFAULT), H5Dclose, "Unable to open an HDF5 dataset");
            write_string(dataset, ATTR_KIND, "value");
            write_string(dataset, ATTR_PATH, node.path.name);
            write_string(dataset, ATTR_NODE_TYPE, node_type_name(node.dtype));
            if (node.dtype == NodeDtype::Link) {
                write_string(dataset, ATTR_LINK_SOURCE, node.link_source);
                write_string(dataset, ATTR_LINK_TARGET, node.link_target);
            }
            write_string(dataset, ATTR_VALUE_TYPE, data_type_name(node.value_dtype));
            write_scalar<uint8_t>(dataset, ATTR_ARRAY, H5T_NATIVE_UINT8, !node.dimension.empty());
            write_scalar<uint8_t>(dataset, ATTR_CONSTANT, H5T_NATIVE_UINT8, node.constant);
            write_scalar<uint64_t>(
                dataset, ATTR_VALUE_ORIGIN, H5T_NATIVE_UINT64, static_cast<uint64_t>(node.value_origin)
            );
            if (!node.condition.empty())
                write_string(dataset, ATTR_CONDITION, node.condition);
            if (!node.format.empty())
                write_string(dataset, ATTR_FORMAT_SPEC, node.format);
            write_strings(dataset, ATTR_TAGS, node.tags);
            std::vector<std::string> option_values;
            std::vector<std::string> option_units;
            std::vector<std::string> option_labels;
            option_values.reserve(node.options.size());
            option_units.reserve(node.options.size());
            option_labels.reserve(node.options.size());
            for (const auto& option : node.options) {
                option_values.push_back(option.value_raw);
                option_units.push_back(option.units_raw);
                option_labels.push_back(option.label);
            }
            write_strings(dataset, ATTR_OPTIONS, option_values);
            write_strings(dataset, ATTR_OPTION_UNITS, option_units);
            if (node.option_labels)
                write_strings(dataset, ATTR_OPTION_LABELS, option_labels);
            write_strings(dataset, ATTR_SCHEMAS, node.schemas);
            if (!node.schema_id.empty())
                write_string(dataset, ATTR_NODE_SCHEMA_ID, node.schema_id);
            if (!node.table_path.empty()) {
                write_string(dataset, ATTR_TABLE_PATH, node.table_path);
                write_scalar<uint64_t>(dataset, ATTR_TABLE_COLUMN_INDEX, H5T_NATIVE_UINT64, node.table_column_index);
            }
            write_scalar<uint8_t>(dataset, ATTR_OVERRIDE, H5T_NATIVE_UINT8, node.override);
            if (node.override) {
                write_string(dataset, ATTR_OVERRIDE_SOURCE, node.override_line.source.name);
                write_scalar<uint64_t>(
                    dataset, ATTR_OVERRIDE_LINE, H5T_NATIVE_UINT64, node.override_line.source.line_number
                );
                write_string(dataset, ATTR_OVERRIDE_CODE, node.override_line.code);
            }
            if (!node.modification_lines.empty()) {
                std::vector<std::string> sources, lines, codes;
                for (const auto& modification : node.modification_lines) {
                    sources.push_back(modification.source.name);
                    lines.push_back(std::to_string(modification.source.line_number));
                    codes.push_back(modification.code);
                }
                write_strings(dataset, ATTR_MODIFICATION_SOURCES, sources);
                write_strings(dataset, ATTR_MODIFICATION_LINES, lines);
                write_strings(dataset, ATTR_MODIFICATION_CODES, codes);
            }
            if (node.units)
                write_string(dataset, "units", node.units->to_string());
            if (!node.line.source.name.empty())
                write_string(dataset, ATTR_SOURCE, node.line.source.name);
            write_scalar<uint64_t>(dataset, ATTR_SOURCE_LINE, H5T_NATIVE_UINT64, node.line.source.line_number);
            if (!node.line.code.empty())
                write_string(dataset, ATTR_SOURCE_CODE, node.line.code);
            write_metadata(dataset, node.metadata);
        }

        bool has_descendant(const Environment& env, const std::string& path) {
            const std::string dot_prefix = path + ".";
            const std::string collection_prefix = path + "[";
            for (const auto& candidate : env.nodes.get_nodes()) {
                if (!candidate)
                    continue;
                const std::string& candidate_path = candidate->path.name;
                if (candidate_path.rfind(dot_prefix, 0) == 0 || candidate_path.rfind(collection_prefix, 0) == 0)
                    return true;
            }
            return false;
        }

        void write_node(hid_t file, const Environment& env, const ValueNode& node) {
            if (!node.schemas.empty())
                throw dip::IOException(
                    "Invalid value-node schema",
                    "The value node `" + node.path.name + "` has applied schemas.",
                    "Apply schemas to a group, map item, or list item rather than to a value node.",
                    __FILE__,
                    __LINE__
                );
            const std::string path = dataset_path(file, env, node.path.name);
            if (node.dtype == NodeDtype::Link) {
                std::string kind = "group";
                const auto open = node.path.name.rfind('[');
                if (open != std::string::npos) {
                    const std::string parent = node.path.name.substr(0, open);
                    if (env.hierarchy.get_collection(parent).kind == Path::Kind::List)
                        kind = "list_item";
                    else
                        kind = "map_item";
                }
                Id group = ensure_group(file, path, kind);
                if (kind == "map_item")
                    write_string(group, ATTR_KEY, node.path.name.substr(open + 1, node.path.name.size() - open - 2));
                if (kind == "list_item")
                    write_scalar<uint64_t>(
                        group, ATTR_INDEX, H5T_NATIVE_UINT64,
                        std::stoull(node.path.name.substr(open + 1, node.path.name.size() - open - 2))
                    );
                write_string(group, ATTR_PATH, node.path.name);
                write_string(group, ATTR_NODE_TYPE, node_type_name(node.dtype));
                write_string(group, ATTR_LINK_SOURCE, node.link_source);
                write_string(group, ATTR_LINK_TARGET, node.link_target);
                write_string(group, ATTR_SOURCE, node.line.source.name);
                write_scalar<uint64_t>(
                    group, ATTR_SOURCE_LINE, H5T_NATIVE_UINT64, static_cast<uint64_t>(node.line.source.line_number)
                );
                if (!node.line.code.empty())
                    write_string(group, ATTR_SOURCE_CODE, node.line.code);
                write_metadata(group, node.metadata);
                return;
            }
            if (!has_descendant(env, node.path.name)) {
                write_value_dataset(file, path, node);
                return;
            }
            Id group = ensure_group(file, path, std::string(KIND_VALUE_GROUP));
            write_string(group, ATTR_PATH, node.path.name);
            write_value_dataset(group, std::string(VALUE_PAYLOAD), node);
        }

        void write_hierarchy_metadata(hid_t file, const Environment& env) {
            for (const auto& [path, collection] : env.hierarchy.get_collections()) {
                if (collection.metadata.present.empty())
                    continue;
                const std::string hdf5_path = dataset_path(file, env, path);
                std::string kind;
                switch (collection.kind) {
                case Path::Kind::Group:
                    kind = "group";
                    break;
                case Path::Kind::Map:
                    kind = "map";
                    break;
                case Path::Kind::List:
                    kind = "list";
                    break;
                case Path::Kind::Item: {
                    const auto open = path.rfind('[');
                    if (open == std::string::npos)
                        throw Error("A metadata-bearing collection item has an invalid path");
                    const auto parent = env.hierarchy.get_collection(path.substr(0, open));
                    kind = parent.kind == Path::Kind::List ? "list_item" : "map_item";
                    break;
                }
                default:
                    continue;
                }
                Id group = ensure_group(file, hdf5_path, kind);
                write_string(group, ATTR_PATH, path);
                write_strings(group, ATTR_SCHEMAS, collection.schemas);
                write_metadata(group, collection.metadata);
            }
        }

        std::vector<hsize_t> dimensions(hid_t dataset) {
            Id space(H5Dget_space(dataset), H5Sclose, "Unable to inspect an HDF5 dataset dataspace");
            const int rank = H5Sget_simple_extent_ndims(space);
            if (rank < 0)
                throw Error("Unable to inspect an HDF5 dataset rank");
            if (rank == 0)
                return {};
            std::vector<hsize_t> dims(rank);
            check(H5Sget_simple_extent_dims(space, dims.data(), nullptr), "Unable to inspect an HDF5 dataset shape");
            return dims;
        }
        size_t element_count(const val::Array::ShapeType& dims) {
            size_t count = 1;
            for (const size_t extent : dims)
                count = checked_product(count, extent, "dataset element count");
            if (count > 100000000)
                throw Error("DIPH5 value exceeds the reader element limit");
            return count;
        }

        void validate_value_type(hid_t dataset, core::DataType dtype) {
            Id type(H5Dget_type(dataset), H5Tclose, "Unable to inspect DIPH5 value type");
            size_t width = 0;
            H5T_class_t category = H5T_INTEGER;
            H5T_sign_t sign = H5T_SGN_2;
            switch (dtype) {
            case core::DataType::Boolean: case core::DataType::Integer8_U: width = 1; sign = H5T_SGN_NONE; break;
            case core::DataType::Character: case core::DataType::Integer8: width = 1; break;
            case core::DataType::Integer16: width = 2; break;
            case core::DataType::Integer32: width = 4; break;
            case core::DataType::Integer64: width = 8; break;
            case core::DataType::Integer16_U: width = 2; sign = H5T_SGN_NONE; break;
            case core::DataType::Integer32_U: width = 4; sign = H5T_SGN_NONE; break;
            case core::DataType::Integer64_U: width = 8; sign = H5T_SGN_NONE; break;
            case core::DataType::Float32: width = 4; category = H5T_FLOAT; break;
            case core::DataType::Float64: width = 8; category = H5T_FLOAT; break;
            case core::DataType::Float128:
                if (H5Tequal(type, H5T_NATIVE_LDOUBLE) <= 0)
                    throw Error("DIPH5 float128 representation does not match native long double");
                return;
            case core::DataType::String:
                if (H5Tget_class(type) != H5T_STRING || H5Tis_variable_str(type) != 0 ||
                    H5Tget_size(type) == 0 || H5Tget_size(type) > 1024 * 1024)
                    throw Error("Invalid DIPH5 fixed-width string type");
                return;
            default: throw Error("Unsupported non-null DIPH5 value type");
            }
            if (H5Tget_class(type) != category || H5Tget_size(type) != width ||
                H5Tget_precision(type) != width * 8 ||
                (category == H5T_INTEGER && H5Tget_sign(type) != sign))
                throw Error("DIPH5 dataset type does not match its declared value type");
            if (category == H5T_FLOAT) {
                const auto little = width == 4 ? H5T_IEEE_F32LE : H5T_IEEE_F64LE;
                const auto big = width == 4 ? H5T_IEEE_F32BE : H5T_IEEE_F64BE;
                if (H5Tequal(type, little) <= 0 && H5Tequal(type, big) <= 0)
                    throw Error("DIPH5 floating-point dataset is not IEEE binary32 or binary64");
            }
        }

        template <typename Disk, typename Stored>
        val::BaseValue::PointerType read_numeric(
            hid_t dataset, hid_t type, core::DataType dtype, const val::Array::ShapeType& shape
        ) {
            std::vector<Disk> disk(element_count(shape));
            check(H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, disk.data()), "Unable to read an HDF5 dataset");
            if (dtype == core::DataType::Boolean)
                for (const auto value : disk)
                    if (value != 0 && value != 1) throw Error("Invalid DIPH5 boolean value");
            std::vector<Stored> stored(disk.begin(), disk.end());
            return std::make_unique<val::ArrayValue<Stored>>(stored, shape, dtype);
        }

        val::BaseValue::PointerType read_value(hid_t dataset, core::DataType dtype) {
            Id space(H5Dget_space(dataset), H5Sclose, "Unable to inspect an HDF5 dataset dataspace");
            if (H5Sget_simple_extent_type(space) == H5S_NULL)
                return nullptr;
            validate_value_type(dataset, dtype);
            const auto disk_dims = dimensions(dataset);
            val::Array::ShapeType shape;
            if (disk_dims.empty())
                shape = {1};
            else
                for (const hsize_t extent : disk_dims)
                    shape.push_back(checked_size(extent, "dataset dimension"));
            switch (dtype) {
            case core::DataType::Boolean:
                return read_numeric<uint8_t, uint8_t>(dataset, H5T_NATIVE_UINT8, dtype, shape);
            case core::DataType::Character:
                return read_numeric<int8_t, int8_t>(dataset, H5T_NATIVE_INT8, dtype, shape);
            case core::DataType::Integer8:
                return read_numeric<int8_t, int64_t>(dataset, H5T_NATIVE_INT8, dtype, shape);
            case core::DataType::Integer16:
                return read_numeric<int16_t, int64_t>(dataset, H5T_NATIVE_INT16, dtype, shape);
            case core::DataType::Integer32:
                return read_numeric<int32_t, int64_t>(dataset, H5T_NATIVE_INT32, dtype, shape);
            case core::DataType::Integer64:
                return read_numeric<int64_t, int64_t>(dataset, H5T_NATIVE_INT64, dtype, shape);
            case core::DataType::Integer8_U:
                return read_numeric<uint8_t, uint64_t>(dataset, H5T_NATIVE_UINT8, dtype, shape);
            case core::DataType::Integer16_U:
                return read_numeric<uint16_t, uint64_t>(dataset, H5T_NATIVE_UINT16, dtype, shape);
            case core::DataType::Integer32_U:
                return read_numeric<uint32_t, uint64_t>(dataset, H5T_NATIVE_UINT32, dtype, shape);
            case core::DataType::Integer64_U:
                return read_numeric<uint64_t, uint64_t>(dataset, H5T_NATIVE_UINT64, dtype, shape);
            case core::DataType::Float32:
                return read_numeric<float, double>(dataset, H5T_NATIVE_FLOAT, dtype, shape);
            case core::DataType::Float64:
                return read_numeric<double, double>(dataset, H5T_NATIVE_DOUBLE, dtype, shape);
            case core::DataType::Float128:
                return read_numeric<long double, long double>(dataset, H5T_NATIVE_LDOUBLE, dtype, shape);
            case core::DataType::String: {
                Id type(H5Dget_type(dataset), H5Tclose, "Unable to inspect an HDF5 string dataset");
                const size_t width = H5Tget_size(type);
                if (width == 0)
                    throw Error("Invalid HDF5 string dataset");
                const size_t count = element_count(shape);
                const auto bytes = checked_product(count, width, "string dataset size");
                if (bytes > 256ULL * 1024 * 1024) throw Error("DIPH5 string dataset exceeds reader limit");
                std::vector<char> buffer(bytes, '\0');
                check(
                    H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()),
                    "Unable to read an HDF5 string dataset"
                );
                std::vector<std::string> values;
                values.reserve(count);
                for (size_t i = 0; i < count; ++i) {
                    const char* begin = buffer.data() + i * width;
                    const char* end = std::find(begin, begin + width, '\0');
                    if (end == begin + width) throw Error("Unterminated DIPH5 string value");
                    values.emplace_back(begin, end);
                    if (!valid_utf8(values.back())) throw Error("Invalid UTF-8 in DIPH5 string value");
                }
                return std::make_unique<val::ArrayValue<std::string>>(values, shape, dtype);
            }
            case core::DataType::None:
                return nullptr;
            default:
                throw dip::IOException(
                    "Unsupported value type",
                    "The persisted value type is not supported.",
                    "Use a file written with a compatible SciNumTools3 version.",
                    __FILE__,
                    __LINE__
                );
            }
        }

        val::BaseValue::PointerType option_value(core::DataType dtype, const std::string& value) {
            switch (dtype) {
            case core::DataType::String:
                return std::make_unique<val::ArrayValueStr>(value);
            case core::DataType::Integer8:
            case core::DataType::Integer16:
            case core::DataType::Integer32:
            case core::DataType::Integer64:
                return std::make_unique<val::ArrayValue<int64_t>>(std::stoll(value), dtype);
            case core::DataType::Integer8_U:
            case core::DataType::Integer16_U:
            case core::DataType::Integer32_U:
            case core::DataType::Integer64_U:
                return std::make_unique<val::ArrayValue<uint64_t>>(std::stoull(value), dtype);
            case core::DataType::Float32:
            case core::DataType::Float64:
                return std::make_unique<val::ArrayValue<double>>(std::stod(value), dtype);
            case core::DataType::Float128:
                return std::make_unique<val::ArrayValue<long double>>(std::stold(value), dtype);
            default:
                return nullptr;
            }
        }

        void read_metadata(hid_t object, ValueMetadata& metadata) {
#define SNT_READ_METADATA(TYPE, FIELD, NAME) metadata.FIELD = read_string(object, NAME);
            SNT_DIP_METADATA_SCALAR_FIELDS(SNT_READ_METADATA)
#undef SNT_READ_METADATA
#define SNT_READ_METADATA_LIST(TYPE, FIELD, NAME) metadata.FIELD = read_strings(object, NAME);
            SNT_DIP_METADATA_LIST_FIELDS(SNT_READ_METADATA_LIST)
#undef SNT_READ_METADATA_LIST
#define SNT_READ_METADATA_PRESENCE(TYPE, FIELD, NAME)                                                                  \
    if (has_attribute(object, NAME) || !metadata.FIELD.empty())                                                        \
        metadata.present.insert(PropertyType::TYPE);
            SNT_DIP_METADATA_SCALAR_FIELDS(SNT_READ_METADATA_PRESENCE)
            SNT_DIP_METADATA_LIST_FIELDS(SNT_READ_METADATA_PRESENCE)
#undef SNT_READ_METADATA_PRESENCE
            for (const auto& name : read_strings(object, "metadata_present")) {
#define SNT_READ_METADATA_NAME(TYPE, FIELD, NAME)                                                                      \
    if (name == NAME)                                                                                                  \
        metadata.present.insert(PropertyType::TYPE);
                SNT_DIP_METADATA_SCALAR_FIELDS(SNT_READ_METADATA_NAME)
                SNT_DIP_METADATA_LIST_FIELDS(SNT_READ_METADATA_NAME)
#undef SNT_READ_METADATA_NAME
            }
        }

        void read_dataset(Environment& env, hid_t dataset) {
            if (read_string(dataset, ATTR_KIND) != "value")
                throw Error("Invalid DIPH5 value kind");
            const std::string path = required_utf8(dataset, ATTR_PATH);
            const core::DataType value_dtype = data_type(required_utf8(dataset, ATTR_VALUE_TYPE));
            const auto array = required_scalar<uint8_t>(dataset, ATTR_ARRAY, H5T_NATIVE_UINT8);
            if (array > 1) throw Error("Invalid DIPH5 array marker");
            Id space(H5Dget_space(dataset), H5Sclose, "Unable to inspect DIPH5 value shape");
            const auto shape_kind = H5Sget_simple_extent_type(space);
            if (shape_kind != H5S_NULL && ((array == 0 && shape_kind != H5S_SCALAR) ||
                                          (array == 1 && shape_kind != H5S_SIMPLE)))
                throw Error("DIPH5 array marker does not match dataspace");
            val::BaseValue::PointerType value = read_value(dataset, value_dtype);
            std::optional<puq::Quantity> units;
            if (has_attribute(dataset, "units"))
                units.emplace(read_string(dataset, "units"));

            ValueNode::PointerType node;
            const NodeDtype dtype = node_type(required_utf8(dataset, ATTR_NODE_TYPE));
            if (value)
                node = std::make_shared<PersistedValueNode>(Path(path), std::move(value), dtype, units);
            else
                node = std::make_shared<PersistedValueNode>(Path(path), value_dtype, dtype);
            if (node->value && array) {
                node->dimension.clear();
                for (const auto extent : node->value->get_shape())
                    node->dimension.push_back({extent, extent});
            }
            const auto constant = required_scalar<uint8_t>(dataset, ATTR_CONSTANT, H5T_NATIVE_UINT8);
            if (constant > 1) throw Error("Invalid DIPH5 constant marker");
            node->constant = constant != 0;
            if (dtype == NodeDtype::Link) {
                node->link_source = read_string(dataset, ATTR_LINK_SOURCE);
                node->link_target = read_string(dataset, ATTR_LINK_TARGET);
                node->value_origin = ValueOrigin::Empty;
            }
            node->condition = read_string(dataset, ATTR_CONDITION);
            node->format = read_string(dataset, ATTR_FORMAT_SPEC);
            node->tags = read_strings(dataset, ATTR_TAGS);
            const auto option_values = read_strings(dataset, ATTR_OPTIONS);
            const auto option_units = read_strings(dataset, ATTR_OPTION_UNITS);
            const auto option_labels = read_strings(dataset, ATTR_OPTION_LABELS);
            if ((has_attribute(dataset, ATTR_OPTION_UNITS) && option_units.size() != option_values.size()) ||
                (has_attribute(dataset, ATTR_OPTION_LABELS) && option_labels.size() != option_values.size()))
                throw Error("DIPH5 option metadata lengths disagree");
            if (has_attribute(dataset, ATTR_OPTION_LABELS))
                node->option_labels = option_labels;
            for (size_t i = 0; i < option_values.size(); ++i) {
                const std::string units_value = i < option_units.size() ? option_units[i] : "";
                const std::string label = i < option_labels.size() ? option_labels[i] : "";
                node->options.push_back(
                    {option_value(value_dtype, option_values[i]), option_values[i], units_value, label}
                );
            }
            node->schemas = read_strings(dataset, ATTR_SCHEMAS);
            node->schema_id = read_string(dataset, ATTR_NODE_SCHEMA_ID);
            node->table_path = read_string(dataset, ATTR_TABLE_PATH);
            if (!node->table_path.empty() && has_attribute(dataset, ATTR_TABLE_COLUMN_INDEX))
                node->table_column_index = checked_size(
                    read_scalar<uint64_t>(dataset, ATTR_TABLE_COLUMN_INDEX, H5T_NATIVE_UINT64), "table column index"
                );
            const auto override_marker = required_scalar<uint8_t>(dataset, ATTR_OVERRIDE, H5T_NATIVE_UINT8);
            if (override_marker > 1) throw Error("Invalid DIPH5 override marker");
            node->override = override_marker != 0;
            if (node->override) {
                node->override_line.source.name = read_string(dataset, ATTR_OVERRIDE_SOURCE);
                node->override_line.source.line_number = checked_size(
                    read_scalar<uint64_t>(dataset, ATTR_OVERRIDE_LINE, H5T_NATIVE_UINT64), "override line number"
                );
                node->override_line.code = read_string(dataset, ATTR_OVERRIDE_CODE);
            }
            const auto modification_sources = read_strings(dataset, ATTR_MODIFICATION_SOURCES);
            const auto modification_lines = read_strings(dataset, ATTR_MODIFICATION_LINES);
            const auto modification_codes = read_strings(dataset, ATTR_MODIFICATION_CODES);
            if (modification_sources.size() != modification_lines.size() ||
                modification_sources.size() != modification_codes.size())
                throw dip::IOException(
                    "Invalid value modification history",
                    "The persisted modification attributes have different lengths for `" + path + "`.",
                    "Use a DIPH5 file with matching modification sources, lines, and codes.",
                    __FILE__,
                    __LINE__
                );
            for (size_t i = 0; i < modification_sources.size(); ++i) {
                try {
                    const size_t line_number =
                        checked_size(std::stoull(modification_lines[i]), "modification line number");
                    node->modification_lines.push_back({modification_codes[i], {modification_sources[i], line_number}});
                } catch (const std::exception&) {
                    throw dip::IOException(
                        "Invalid value modification history",
                        "A persisted modification line number is invalid for `" + path + "`.",
                        "Use a DIPH5 file with valid modification line numbers.",
                        __FILE__,
                        __LINE__
                    );
                }
            }
            if (!node->schemas.empty())
                throw dip::IOException(
                    "Invalid value-node schema",
                    "The persisted value node `" + path + "` has applied schemas.",
                    "Use a DIPH5 file where schemas are applied only to groups or collection items.",
                    __FILE__,
                    __LINE__
                );
            const auto origin = required_scalar<uint64_t>(dataset, ATTR_VALUE_ORIGIN, H5T_NATIVE_UINT64);
            if (origin > static_cast<uint64_t>(ValueOrigin::Schema)) throw Error("Invalid DIPH5 value origin");
            node->value_origin = static_cast<ValueOrigin>(origin);
            node->line.source.name = read_string(dataset, ATTR_SOURCE);
            node->line.source.line_number =
                checked_size(read_scalar<uint64_t>(dataset, ATTR_SOURCE_LINE, H5T_NATIVE_UINT64), "source line number");
            node->line.code = read_string(dataset, ATTR_SOURCE_CODE);
            read_metadata(dataset, node->metadata);
            env.nodes.push_back(node);
            if (!env.hierarchy.has_collection(path))
                env.hierarchy.set_collection(path, Path::Kind::Group, node->schemas);
        }

        struct ChildObject {
            std::string name;
            int type = 0;
            bool semantic = false;
            std::optional<uint64_t> order;
        };

        std::vector<ChildObject> ordered_children(hid_t group) {
            std::vector<ChildObject> children;
            const bool ordered_list = read_string(group, ATTR_KIND) == "list";
            const hsize_t count = object_count(group);
            children.reserve(checked_size(count, "child count"));
            for (hsize_t index = 0; index < count; ++index) {
                ChildObject child;
                child.name = object_name(group, index);
                child.type = H5Gget_objtype_by_idx(group, index);
                if (child.type < 0)
                    throw Error("Unable to inspect an HDF5 child object");
                if (child.name != std::string(VALUE_PAYLOAD)) {
                    Id object(H5Oopen(group, child.name.c_str(), H5P_DEFAULT), H5Oclose,
                              "Unable to inspect a DIPH5 child object");
                    child.semantic = has_attribute(object, ATTR_PATH);
                    if (child.semantic && ordered_list && child.type == H5G_GROUP &&
                        read_string(object, ATTR_KIND) == "list_item")
                        child.order = read_scalar<uint64_t>(object, ATTR_INDEX, H5T_NATIVE_UINT64);
                    else if (child.semantic && has_attribute(object, ATTR_ORDER))
                        child.order = read_scalar<uint64_t>(object, ATTR_ORDER, H5T_NATIVE_UINT64);
                }
                children.push_back(std::move(child));
            }

            bool complete_order = std::all_of(children.begin(), children.end(), [](const auto& child) {
                return !child.semantic || child.order.has_value();
            });
            if (complete_order) {
                std::vector<uint64_t> positions;
                for (const auto& child : children)
                    if (child.semantic)
                        positions.push_back(*child.order);
                std::sort(positions.begin(), positions.end());
                for (size_t index = 0; index < positions.size(); ++index)
                    if (positions[index] != index) {
                        complete_order = false;
                        break;
                    }
            }
            std::stable_sort(children.begin(), children.end(), [complete_order](const auto& left, const auto& right) {
                if (left.semantic != right.semantic)
                    return left.semantic;
                if (!left.semantic)
                    return left.name < right.name;
                if (complete_order && left.order != right.order)
                    return left.order < right.order;
                return left.name < right.name;
            });
            return children;
        }

        void read_trace_manifest(Environment& env, hid_t file) {
            const std::string manifest_path = "/" + std::string(GROUP_TRACE);
            const htri_t exists = H5Lexists(file, manifest_path.c_str(), H5P_DEFAULT);
            if (exists < 0)
                throw Error("Unable to inspect the HDF5 trace manifest");
            if (exists == 0)
                throw dip::IOException(
                    "Invalid HDF5 trace manifest",
                    "The DIPH5 v3 resolved tree is missing its trace manifest.",
                    "Use a complete DIPH5 file written by a compatible SciNumTools3 version.",
                    __FILE__,
                    __LINE__
                );

            Id manifest(
                H5Gopen2(file, manifest_path.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open the HDF5 trace manifest"
            );
            std::vector<TraceInfo> traces;
            std::vector<SchemaInfo> schema_infos;
            const hsize_t count = object_count(manifest);
            traces.reserve(checked_size(count, "trace-manifest count"));
            for (hsize_t index = 0; index < count; ++index) {
                const std::string entry_name = object_name(manifest, index);
                if (H5Gget_objtype_by_idx(manifest, index) != H5G_GROUP)
                    throw dip::IOException(
                        "Invalid HDF5 trace manifest",
                        "A trace-manifest entry is not an HDF5 group.",
                        "Use a complete DIPH5 file written by a compatible SciNumTools3 version.",
                        __FILE__,
                        __LINE__
                    );
                Id entry(
                    H5Gopen2(manifest, entry_name.c_str(), H5P_DEFAULT),
                    H5Gclose,
                    "Unable to open an HDF5 trace-manifest entry"
                );
                TraceInfo trace{
                    read_string(entry, ATTR_TRACE_ID),
                    read_string(entry, ATTR_TRACE_NAME),
                    read_string(entry, ATTR_TRACE_KIND)
                };
                if (trace.id.empty() || trace.name.empty() || trace.kind.empty())
                    throw dip::IOException(
                        "Invalid HDF5 trace manifest",
                        "A trace-manifest entry is missing its identifier, name, or kind.",
                        "Use a complete DIPH5 file written by a compatible SciNumTools3 version.",
                        __FILE__,
                        __LINE__
                    );
                if (trace.kind == "schema") {
                    SchemaInfo info;
                    info.id = trace.id;
                    info.name = trace.name;
                    {
                        info.source_name = read_string(entry, ATTR_SCHEMA_SOURCE);
                        info.source_line = checked_size(
                            read_scalar<uint64_t>(entry, ATTR_SCHEMA_SOURCE_LINE, H5T_NATIVE_UINT64),
                            "schema source line number"
                        );
                        read_metadata(entry, info.metadata);
                    }
                    schema_infos.push_back(std::move(info));
                }
                traces.push_back(std::move(trace));
            }
            env.set_trace_manifest(std::move(traces));
            env.set_schema_manifest(std::move(schema_infos));
        }

        void read_unit_manifest(Environment& env, hid_t file) {
            const std::string manifest_path = "/" + std::string(GROUP_UNITS);
            const htri_t exists = H5Lexists(file, manifest_path.c_str(), H5P_DEFAULT);
            if (exists < 0)
                throw Error("Unable to inspect the HDF5 unit manifest");
            if (exists == 0)
                throw dip::IOException(
                    "Invalid HDF5 unit manifest",
                    "The DIPH5 v3 resolved tree is missing its custom-unit manifest.",
                    "Use a complete DIPH5 v3 archive.",
                    __FILE__,
                    __LINE__
                );
            Id manifest(
                H5Gopen2(file, manifest_path.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open the HDF5 unit manifest"
            );
            struct Record {
                std::string name;
                std::string definition;
                std::string id;
                uint64_t order;
            };
            std::vector<Record> records;
            std::set<std::string> names;
            std::set<std::string> ids;
            std::set<uint64_t> orders;
            const hsize_t count = object_count(manifest);
            records.reserve(checked_size(count, "unit-manifest count"));
            for (hsize_t index = 0; index < count; ++index) {
                const std::string entry_name = object_name(manifest, index);
                if (H5Gget_objtype_by_idx(manifest, index) != H5G_GROUP)
                    throw dip::IOException(
                        "Invalid HDF5 unit manifest",
                        "A unit-manifest entry is not an HDF5 group.",
                        "Use a complete DIPH5 v3 archive.",
                        __FILE__,
                        __LINE__
                    );
                Id entry(
                    H5Gopen2(manifest, entry_name.c_str(), H5P_DEFAULT),
                    H5Gclose,
                    "Unable to open an HDF5 unit-manifest entry"
                );
                Record record{
                    read_string(entry, ATTR_UNIT_NAME),
                    read_string(entry, ATTR_UNIT_DEFINITION),
                    read_string(entry, ATTR_TRACE_ID),
                    read_scalar<uint64_t>(entry, ATTR_UNIT_ORDER, H5T_NATIVE_UINT64)
                };
                if (record.name.empty() || record.definition.empty() || record.id.empty() ||
                    !names.insert(record.name).second || !ids.insert(record.id).second ||
                    !orders.insert(record.order).second)
                    throw dip::IOException(
                        "Invalid HDF5 unit manifest",
                        "A unit-manifest entry has missing or duplicate identity data.",
                        "Use a complete DIPH5 v3 archive.",
                        __FILE__,
                        __LINE__
                    );
                const auto traces = env.get_trace_manifest();
                const auto trace = std::find_if(traces.begin(), traces.end(), [&](const TraceInfo& item) {
                    return item.id == record.id;
                });
                if (trace == traces.end() || trace->name != record.name || trace->kind != "unit")
                    throw dip::IOException(
                        "Invalid HDF5 unit manifest",
                        "A unit-manifest trace identifier does not match the trace manifest.",
                        "Use a complete DIPH5 v3 archive.",
                        __FILE__,
                        __LINE__
                    );
                records.push_back(std::move(record));
            }
            std::sort(records.begin(), records.end(), [](const Record& left, const Record& right) {
                return left.order < right.order;
            });
            for (const auto& record : records) {
                try {
                    env.units.append(
                        record.name,
                        EnvUnit{
                            record.name,
                            record.definition,
                            0,
                            record.id,
                            checked_size(record.order, "unit registration order")
                        }
                    );
                } catch (const std::exception& error) {
                    throw dip::IOException(
                        "Invalid HDF5 unit manifest",
                        "Unable to register custom unit `" + record.name + "`: " + error.what(),
                        "Ensure unit definitions are valid and ordered by dependency.",
                        __FILE__,
                        __LINE__
                    );
                }
            }
        }

        void read_group(
            Environment& env,
            hid_t group,
            bool root = false,
            bool skip_value_payload = false
        ) {
            const auto children = ordered_children(group);
            for (const auto& child_object : children) {
                const std::string& name = child_object.name;
                if (root && (name == std::string(GROUP_TRACE) || name == std::string(GROUP_UNITS) ||
                             name == std::string(GROUP_DEPENDENCIES) || name == std::string(GROUP_OUTPUTS)))
                    continue;
                if (skip_value_payload && name == VALUE_PAYLOAD)
                    continue;
                const int type = child_object.type;
                if (type == H5G_GROUP) {
                    Id child(H5Gopen2(group, name.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open an HDF5 group");
                    const std::string kind = read_string(child, ATTR_KIND);
                    const std::string path = read_string(child, ATTR_PATH);
                    if (!path.empty())
                        env.hierarchy.record_ordered_path(path);
                    if (read_string(child, ATTR_NODE_TYPE) == "link") {
                        auto link = std::make_shared<PersistedValueNode>(
                            Path(path), core::DataType::None, NodeDtype::Link
                        );
                        link->link_source = read_string(child, ATTR_LINK_SOURCE);
                        link->link_target = read_string(child, ATTR_LINK_TARGET);
                        link->value_origin = ValueOrigin::Empty;
                        link->line.source.name = read_string(child, ATTR_SOURCE);
                        link->line.source.line_number = checked_size(
                            read_scalar<uint64_t>(child, ATTR_SOURCE_LINE, H5T_NATIVE_UINT64), "source line number"
                        );
                        link->line.code = read_string(child, ATTR_SOURCE_CODE);
                        read_metadata(child, link->metadata);
                        env.nodes.push_back(link);
                    }
                    if (kind == KIND_VALUE_GROUP) {
                        const htri_t payload_exists = H5Lexists(child, std::string(VALUE_PAYLOAD).c_str(), H5P_DEFAULT);
                        if (payload_exists <= 0)
                            throw dip::IOException(
                                "Invalid DIPH5 value group",
                                "The value group at `" + path + "` does not contain its `_DIPL_Value` payload.",
                                "Use a complete DIPH5 v3 archive.",
                                __FILE__,
                                __LINE__
                            );
                        Id payload(
                            H5Dopen2(child, std::string(VALUE_PAYLOAD).c_str(), H5P_DEFAULT),
                            H5Dclose,
                            "Unable to open a DIPH5 value-group payload"
                        );
                        if (read_string(payload, ATTR_KIND) != "value" || read_string(payload, ATTR_PATH) != path)
                            throw dip::IOException(
                                "Invalid DIPH5 value group",
                                "The value-group payload does not describe its parent DIPL value path.",
                                "Use a complete DIPH5 v3 archive.",
                                __FILE__,
                                __LINE__
                            );
                        read_dataset(env, payload);
                        read_group(env, child, false, true);
                        continue;
                    }
                    if (!path.empty() &&
                        (kind == "group" || kind == "item" || kind == "map_item" || kind == "list_item") &&
                        !env.hierarchy.has_collection(path))
                        env.hierarchy.set_collection(
                            path,
                            kind == "group" ? Path::Kind::Group : Path::Kind::Item,
                            read_strings(child, ATTR_SCHEMAS)
                        );
                    if (!path.empty() && (kind == "map" || kind == "list") && !env.hierarchy.has_collection(path)) {
                        std::vector<std::string> items;
                        for (const auto& item_object : ordered_children(child)) {
                            if (item_object.type != H5G_GROUP)
                                continue;
                            Id item(
                                H5Gopen2(child, item_object.name.c_str(), H5P_DEFAULT),
                                H5Gclose,
                                "Unable to open an HDF5 collection item"
                            );
                            const std::string item_kind = read_string(item, ATTR_KIND);
                            if (kind == "map" && item_kind == "map_item")
                                items.push_back(read_string(item, ATTR_KEY));
                            else if (kind == "list" && item_kind == "list_item")
                                items.push_back(
                                    std::to_string(read_scalar<uint64_t>(item, ATTR_INDEX, H5T_NATIVE_UINT64))
                                );
                        }
                        if (kind == "list")
                            std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
                                return std::stoull(a) < std::stoull(b);
                            });
                        env.hierarchy.set_collection(
                            path,
                            kind == "map" ? Path::Kind::Map : Path::Kind::List,
                            read_strings(child, ATTR_SCHEMAS),
                            items
                        );
                    }
                    if (!path.empty() && (kind == "group" || kind == "item" || kind == "map_item" ||
                                          kind == "list_item" || kind == "map" || kind == "list")) {
                        ValueMetadata metadata;
                        read_metadata(child, metadata);
                        env.hierarchy.merge_metadata(path, metadata);
                    }
                    read_group(env, child);
                } else if (type == H5G_DATASET) {
                    Id dataset(H5Dopen2(group, name.c_str(), H5P_DEFAULT), H5Dclose, "Unable to open an HDF5 dataset");
                    const std::string path = read_string(dataset, ATTR_PATH);
                    if (!path.empty())
                        env.hierarchy.record_ordered_path(path);
                    read_dataset(env, dataset);
                }
            }
        }
        void write_trace_manifest(hid_t file, const Environment& env) {
            const std::string manifest_path = "/" + std::string(GROUP_TRACE);
            Id manifest(
                H5Gcreate2(file, manifest_path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                H5Gclose,
                "Unable to create the HDF5 trace manifest"
            );
            const auto traces = env.get_trace_manifest();
            const auto schema_infos = env.get_schema_manifest();
            for (size_t index = 0; index < traces.size(); ++index) {
                const TraceInfo& trace = traces[index];
                const std::string entry_name = std::to_string(index);
                Id entry(
                    H5Gcreate2(manifest, entry_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Gclose,
                    "Unable to create an HDF5 trace-manifest entry"
                );
                write_string(entry, ATTR_TRACE_ID, trace.id);
                write_string(entry, ATTR_TRACE_NAME, trace.name);
                write_string(entry, ATTR_TRACE_KIND, trace.kind);
                if (trace.kind == "schema") {
                    const auto info =
                        std::find_if(schema_infos.begin(), schema_infos.end(), [&](const SchemaInfo& candidate) {
                            return candidate.id == trace.id;
                        });
                    if (info != schema_infos.end()) {
                        write_string(entry, ATTR_SCHEMA_SOURCE, info->source_name);
                        write_scalar<uint64_t>(entry, ATTR_SCHEMA_SOURCE_LINE, H5T_NATIVE_UINT64, info->source_line);
                        write_metadata(entry, info->metadata);
                    }
                }
            }
        }

        void write_unit_manifest(hid_t file, const Environment& env) {
            const std::string manifest_path = "/" + std::string(GROUP_UNITS);
            Id manifest(
                H5Gcreate2(file, manifest_path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                H5Gclose,
                "Unable to create the HDF5 unit manifest"
            );
            std::vector<const EnvUnit*> units;
            units.reserve(env.units.entries().size());
            for (const auto& entry : env.units.entries())
                units.push_back(&entry.second);
            std::sort(units.begin(), units.end(), [](const EnvUnit* left, const EnvUnit* right) {
                return left->registration_order < right->registration_order;
            });
            for (size_t index = 0; index < units.size(); ++index) {
                const EnvUnit& unit = *units[index];
                Id entry(
                    H5Gcreate2(manifest, std::to_string(index).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Gclose,
                    "Unable to create an HDF5 unit-manifest entry"
                );
                write_string(entry, ATTR_UNIT_NAME, unit.name);
                write_string(entry, ATTR_UNIT_DEFINITION, unit.definition);
                write_scalar<uint64_t>(entry, ATTR_UNIT_ORDER, H5T_NATIVE_UINT64, unit.registration_order);
                write_string(entry, ATTR_TRACE_ID, unit.id);
            }
        }

        void write_dependency_graph(hid_t file, const DependencyGraph& graph) {
            const std::string path = "/" + std::string(GROUP_DEPENDENCIES);
            Id root(
                H5Gcreate2(file, path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                H5Gclose,
                "Unable to create the DIP dependency graph"
            );
            write_scalar<uint8_t>(root, "recorded", H5T_NATIVE_UINT8, graph.recorded ? 1 : 0);
            for (size_t index = 0; index < graph.events.size(); ++index) {
                const auto& event = graph.events[index];
                Id entry(
                    H5Gcreate2(root, std::to_string(index).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Gclose,
                    "Unable to create a DIP dependency event"
                );
                write_string(entry, "owner", event.owner);
                write_scalar<uint64_t>(entry, "kind", H5T_NATIVE_UINT64, static_cast<uint64_t>(event.kind));
                if (event.location) {
                    write_string(entry, "source", event.location->source);
                    write_scalar<uint64_t>(entry, "line", H5T_NATIVE_UINT64, event.location->line);
                    if (event.location->code)
                        write_string(entry, "code", *event.location->code);
                }
                write_strings(entry, "controlled_by", event.controlled_by);
                write_string(entry, "expression", event.expression);
                std::vector<std::string> targets, requests, operands;
                for (const auto& edge : event.reads) {
                    targets.push_back(edge.target);
                    requests.push_back(edge.request);
                    operands.push_back(edge.operand);
                }
                write_strings(entry, "targets", targets);
                write_strings(entry, "requests", requests);
                write_strings(entry, "operands", operands);
                if (!event.composition)
                    continue;
                write_scalar<uint64_t>(entry, "root", H5T_NATIVE_UINT64, event.composition->root);
                Id composition(
                    H5Gcreate2(entry, "composition", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Gclose,
                    "Unable to create a DIP operation tree"
                );
                for (size_t node_index = 0; node_index < event.composition->nodes.size(); ++node_index) {
                    const auto& node = event.composition->nodes[node_index];
                    Id item(
                        H5Gcreate2(
                            composition, std::to_string(node_index).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT
                        ),
                        H5Gclose,
                        "Unable to create an operation node"
                    );
                    write_scalar<uint64_t>(item, "kind", H5T_NATIVE_UINT64, static_cast<uint64_t>(node.kind));
                    write_string(item, "text", node.text);
                    write_scalar<int64_t>(item, "operator", H5T_NATIVE_INT64, node.operator_type);
                    if (node.operation)
                        write_scalar<int64_t>(
                            item, "operation", H5T_NATIVE_INT64, static_cast<int64_t>(*node.operation)
                        );
                    std::vector<std::string> children;
                    for (auto child : node.children)
                        children.push_back(std::to_string(child));
                    write_strings(item, "children", children);
                }
            }
        }

        void read_dependency_graph(Environment& env, hid_t file) {
            const std::string path = "/" + std::string(GROUP_DEPENDENCIES);
            if (H5Lexists(file, path.c_str(), H5P_DEFAULT) <= 0)
                throw Error("The DIPH5 v3 resolved tree is missing its DIP dependency graph");
            Id root(H5Gopen2(file, path.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open the DIP dependency graph");
            DependencyGraph graph;
            graph.recorded = read_scalar<uint8_t>(root, "recorded", H5T_NATIVE_UINT8, 1) != 0;
            const size_t count = checked_size(object_count(root), "dependency event count");
            for (size_t index = 0; index < count; ++index) {
                Id entry(
                    H5Gopen2(root, std::to_string(index).c_str(), H5P_DEFAULT),
                    H5Gclose,
                    "Unable to open a DIP dependency event"
                );
                DependencyEvent event;
                event.owner = read_string(entry, "owner");
                const auto kind = read_scalar<uint64_t>(entry, "kind", H5T_NATIVE_UINT64);
                if (event.owner.empty() || kind > static_cast<uint64_t>(DependencyEventKind::Decision))
                    throw Error("Invalid DIP dependency event");
                event.kind = static_cast<DependencyEventKind>(kind);
                if (has_attribute(entry, "source"))
                    event.location = core::SourceLocation{
                        read_string(entry, "source"),
                        checked_size(
                            read_scalar<uint64_t>(entry, "line", H5T_NATIVE_UINT64), "dependency event line number"
                        ),
                        has_attribute(entry, "code") ? std::optional<std::string>(read_string(entry, "code"))
                                                     : std::nullopt
                    };
                event.controlled_by = read_strings(entry, "controlled_by");
                event.expression = read_string(entry, "expression");
                const auto targets = read_strings(entry, "targets");
                const auto requests = read_strings(entry, "requests");
                const auto operands = read_strings(entry, "operands");
                if (targets.size() != requests.size() || targets.size() != operands.size())
                    throw Error("Invalid DIP dependency edges");
                for (size_t edge = 0; edge < targets.size(); ++edge)
                    event.reads.push_back({targets[edge], requests[edge], operands[edge]});
                if (H5Lexists(entry, "composition", H5P_DEFAULT) > 0) {
                    Id composition(
                        H5Gopen2(entry, "composition", H5P_DEFAULT), H5Gclose, "Unable to open a DIP operation tree"
                    );
                    event.composition.emplace();
                    event.composition->root =
                        checked_size(read_scalar<uint64_t>(entry, "root", H5T_NATIVE_UINT64), "operation root index");
                    const size_t node_count = checked_size(object_count(composition), "operation node count");
                    for (size_t node_index = 0; node_index < node_count; ++node_index) {
                        Id item(
                            H5Gopen2(composition, std::to_string(node_index).c_str(), H5P_DEFAULT),
                            H5Gclose,
                            "Unable to open a DIP operation node"
                        );
                        exs::CompositionNode node;
                        const auto node_kind = read_scalar<uint64_t>(item, "kind", H5T_NATIVE_UINT64);
                        if (node_kind > static_cast<uint64_t>(exs::CompositionKind::Group))
                            throw Error("Invalid DIP operation node kind");
                        node.kind = static_cast<exs::CompositionKind>(node_kind);
                        node.text = read_string(item, "text");
                        node.operator_type = read_scalar<int64_t>(item, "operator", H5T_NATIVE_INT64);
                        if (has_attribute(item, "operation"))
                            node.operation = static_cast<exs::OperationType>(
                                read_scalar<int64_t>(item, "operation", H5T_NATIVE_INT64)
                            );
                        for (const auto& child : read_strings(item, "children")) {
                            size_t parsed = 0;
                            const auto value = std::stoull(child, &parsed);
                            if (parsed != child.size() || value >= node_count)
                                throw Error("Invalid DIP operation child");
                            node.children.push_back(checked_size(value, "operation child index"));
                        }
                        event.composition->nodes.push_back(std::move(node));
                    }
                    if (event.composition->nodes.empty() || event.composition->root >= node_count)
                        throw Error("Invalid DIP operation root");
                }
                graph.events.push_back(std::move(event));
            }
            env.set_dependency_graph(std::move(graph));
        }

        void write_output_plan(hid_t file, const OutputPlan& plan) {
            const std::string root = "/" + std::string(GROUP_OUTPUTS);
            Id group(
                H5Gcreate2(file, root.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                H5Gclose,
                "Unable to create the DIPH5 output plan"
            );
            for (size_t index = 0; index < plan.mappings().size(); ++index) {
                const auto& mapping = plan.mappings()[index];
                const std::string path = root + "/" + std::to_string(index);
                Id entry(
                    H5Gcreate2(file, path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Gclose,
                    "Unable to create a DIPH5 output mapping"
                );
                write_string(entry, "id", mapping.id);
                write_string(entry, "target", mapping.target);
                write_string(entry, "key", mapping.key);
                write_string(entry, "source_path", mapping.source_path);
                write_string(entry, "rule", mapping.rule);
                write_string(entry, "origin", mapping.origin);
                write_string(entry, "value_path", mapping.value->path.name);
                write_string(entry, ATTR_NODE_TYPE, node_type_name(mapping.value->dtype));
                write_string(entry, ATTR_VALUE_TYPE, data_type_name(mapping.value->value_dtype));
                write_strings(entry, "replacements", mapping.replacements);
                write_strings(entry, "dependencies", mapping.dependencies);
                write_scalar<uint8_t>(entry, "active", H5T_NATIVE_UINT8, mapping.active);
                const std::string value_path = path + "/value";
                write_value(file, value_path, *mapping.value);
                if (mapping.value->units)
                    write_string(entry, "units", mapping.value->units->to_string());
            }
        }

        void read_output_plan(Environment& env, hid_t file) {
            const std::string root = "/" + std::string(GROUP_OUTPUTS);
            const htri_t exists = H5Lexists(file, root.c_str(), H5P_DEFAULT);
            if (exists < 0)
                throw Error("Unable to inspect the DIPH5 output plan");
            if (exists == 0)
                return;
            Id group(H5Gopen2(file, root.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open the DIPH5 output plan");
            OutputPlan plan;
            const hsize_t count = object_count(group);
            for (hsize_t index = 0; index < count; ++index) {
                const std::string name = std::to_string(index);
                Id entry(H5Gopen2(group, name.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open a DIPH5 output mapping");
                OutputMapping mapping;
                mapping.id = read_string(entry, "id");
                mapping.target = read_string(entry, "target");
                mapping.key = read_string(entry, "key");
                mapping.source_path = read_string(entry, "source_path");
                mapping.rule = read_string(entry, "rule");
                mapping.origin = read_string(entry, "origin");
                mapping.replacements = read_strings(entry, "replacements");
                mapping.dependencies = read_strings(entry, "dependencies");
                mapping.active = read_scalar<uint8_t>(entry, "active", H5T_NATIVE_UINT8) != 0;
                Id dataset(H5Dopen2(entry, "value", H5P_DEFAULT), H5Dclose, "Unable to open a DIPH5 output value");
                const auto dtype = data_type(read_string(entry, ATTR_VALUE_TYPE));
                auto value = read_value(dataset, dtype);
                if (!value)
                    throw Error("A DIPH5 output mapping has no value");
                std::optional<puq::Quantity> units;
                if (has_attribute(entry, "units"))
                    units.emplace(read_string(entry, "units"));
                const std::string value_path = read_string(entry, "value_path");
                mapping.value = std::make_shared<PersistedValueNode>(
                    Path(value_path.empty() ? "output_value" : value_path),
                    std::move(value),
                    node_type(read_string(entry, ATTR_NODE_TYPE)),
                    std::move(units)
                );
                plan.restore(std::move(mapping));
            }
            plan.validate();
            env.set_output_plan(std::move(plan));
        }
        bool uses_reserved_path(const std::string& path) {
            for (const auto name :
                 {std::string(GROUP_TRACE),
                  std::string(GROUP_UNITS),
                  std::string(GROUP_DEPENDENCIES),
                  std::string(GROUP_OUTPUTS)}) {
                if (path == name || path.rfind(name + ".", 0) == 0 || path.rfind(name + "[", 0) == 0)
                    return true;
            }
            return false;
        }

        bool uses_value_payload_child(const Environment& env, const std::string& path) {
            const std::string payload_path = path + "." + std::string(VALUE_PAYLOAD);
            for (const auto& candidate : env.nodes.get_nodes())
                if (candidate && candidate->path.name == payload_path)
                    return true;
            return false;
        }

        Id temporary_file() {
            static std::atomic<unsigned long long> sequence{0};
            Id access(H5Pcreate(H5P_FILE_ACCESS), H5Pclose, "Unable to create HDF5 memory access properties");
            check(H5Pset_fapl_core(access, 64 * 1024, false), "Unable to configure HDF5 memory staging");
            const auto name = "snt-diph5-v3-stage-" + std::to_string(sequence++) + ".h5";
            return Id(H5Fcreate(name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, access), H5Fclose,
                      "Unable to create an in-memory HDF5 staging file");
        }

        void write_bytes(hid_t group, const std::string& content) {
            if (content.size() > 256ULL * 1024 * 1024)
                throw Error("Archived source exceeds the 256 MiB archive limit");
            const hsize_t length = content.size();
            Id space(H5Screate_simple(1, &length, nullptr), H5Sclose, "Unable to create source byte space");
            Id dataset(H5Dcreate2(group, "content", H5T_STD_U8LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                       H5Dclose, "Unable to create archived source content");
            if (!content.empty())
                check(H5Dwrite(dataset, H5T_NATIVE_UINT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, content.data()),
                      "Unable to write archived source content");
        }

        std::string read_bytes(hid_t group) {
            Id dataset(H5Dopen2(group, "content", H5P_DEFAULT), H5Dclose, "Missing archived source content");
            Id type(H5Dget_type(dataset), H5Tclose, "Unable to inspect archived content type");
            Id space(H5Dget_space(dataset), H5Sclose, "Unable to inspect archived content shape");
            if (H5Tget_class(type) != H5T_INTEGER || H5Tget_size(type) != 1 ||
                H5Tget_sign(type) != H5T_SGN_NONE ||
                H5Sget_simple_extent_ndims(space) != 1)
                throw Error("Archived content must be a one-dimensional uint8 dataset");
            hsize_t length = 0;
            check(H5Sget_simple_extent_dims(space, &length, nullptr), "Unable to read archived content length");
            if (length > 256 * 1024 * 1024)
                throw Error("Archived source exceeds the 256 MiB reader limit");
            std::string content(checked_size(length, "source byte count"), '\0');
            if (!content.empty())
                check(H5Dread(dataset, H5T_NATIVE_UINT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, content.data()),
                      "Unable to read archived source content");
            return content;
        }

        void copy_object(hid_t source, const std::string& from, hid_t target, const std::string& to) {
            check(H5Ocopy(source, from.c_str(), target, to.c_str(), H5P_DEFAULT, H5P_DEFAULT),
                  "Unable to relocate DIPH5 object");
        }

        void validate_metadata_utf8(const ValueMetadata& metadata) {
#define SNT_CHECK_METADATA_UTF8(TYPE, FIELD, NAME)                                                                     \
    if (!valid_utf8(metadata.FIELD))                                                                                   \
        throw Error("Invalid UTF-8 in DIPH5 v3 metadata: " NAME);
            SNT_DIP_METADATA_SCALAR_FIELDS(SNT_CHECK_METADATA_UTF8)
#undef SNT_CHECK_METADATA_UTF8
#define SNT_CHECK_METADATA_LIST_UTF8(TYPE, FIELD, NAME)                                                                \
    for (const auto& value : metadata.FIELD)                                                                           \
        if (!valid_utf8(value))                                                                                         \
            throw Error("Invalid UTF-8 in DIPH5 v3 metadata: " NAME);
            SNT_DIP_METADATA_LIST_FIELDS(SNT_CHECK_METADATA_LIST_UTF8)
#undef SNT_CHECK_METADATA_LIST_UTF8
        }

        void validate_node_order(hid_t group, size_t depth = 0) {
            if (depth > 256) throw Error("DIPH5 resolved hierarchy exceeds depth limit");
            const bool parent_is_list = read_string(group, ATTR_KIND) == "list";
            std::vector<uint64_t> positions;
            for (hsize_t index = 0; index < object_count(group); ++index) {
                const auto name = object_name(group, index);
                H5L_info_t link_info{};
                check(H5Lget_info(group, name.c_str(), &link_info, H5P_DEFAULT), "Unable to inspect DIPH5 link");
                if (link_info.type != H5L_TYPE_HARD)
                    throw Error("DIPH5 resolved hierarchy must not contain HDF5 soft or external links");
                if (name == VALUE_PAYLOAD) {
                    if (read_string(group, ATTR_KIND) != KIND_VALUE_GROUP)
                        throw Error("Unexpected DIPH5 value-group payload");
                    continue;
                }
                Id child(H5Oopen(group, name.c_str(), H5P_DEFAULT), H5Oclose,
                         "Unable to inspect a DIPH5 v3 resolved node");
                if (!has_attribute(child, ATTR_PATH))
                    throw Error("DIPH5 v3 resolved node has no logical path");
                const auto logical = required_utf8(child, ATTR_PATH);
                const auto expected_path = "/configuration/resolved" + hdf5_path(logical);
                const auto length = H5Iget_name(child, nullptr, 0);
                if (logical.empty() || length < 0) throw Error("Invalid DIPH5 resolved node path");
                std::string actual(static_cast<size_t>(length) + 1, '\0');
                if (H5Iget_name(child, actual.data(), actual.size()) != length)
                    throw Error("Unable to inspect DIPH5 resolved node path");
                actual.resize(static_cast<size_t>(length));
                if (actual != expected_path) throw Error("DIPH5 logical and HDF5 paths disagree");
                const bool list_item = parent_is_list && read_string(child, ATTR_KIND) == "list_item";
                const auto field = list_item ? ATTR_INDEX : ATTR_ORDER;
                if (!has_attribute(child, field))
                    throw Error("DIPH5 v3 resolved node has no order");
                positions.push_back(required_scalar<uint64_t>(child, field, H5T_NATIVE_UINT64));
                if (H5Iget_type(child) == H5I_GROUP)
                    validate_node_order(child, depth + 1);
            }
            std::sort(positions.begin(), positions.end());
            for (size_t index = 0; index < positions.size(); ++index)
                if (positions[index] != index)
                    throw Error("DIPH5 v3 resolved node order is not contiguous");
        }

        void complete_resolved_order(hid_t group) {
            const bool parent_is_list = read_string(group, ATTR_KIND) == "list";
            const auto all_children = checked_size(object_count(group), "resolved child count");
            size_t count = 0;
            for (size_t index = 0; index < all_children; ++index)
                count += object_name(group, index) != VALUE_PAYLOAD;
            std::vector<Id> children;
            std::vector<bool> used(count, false);
            for (size_t index = 0; index < all_children; ++index) {
                const auto name = object_name(group, index);
                if (name == VALUE_PAYLOAD)
                    continue;
                Id child(H5Oopen(group, name.c_str(), H5P_DEFAULT), H5Oclose,
                         "Unable to inspect a resolved node");
                const bool list_item = parent_is_list && read_string(child, ATTR_KIND) == "list_item";
                const auto field = list_item ? ATTR_INDEX : ATTR_ORDER;
                if (has_attribute(child, field)) {
                    const auto position = required_scalar<uint64_t>(child, field, H5T_NATIVE_UINT64);
                    if (position >= count || used[position])
                        throw Error("Invalid resolved node order while saving DIPH5 v3");
                    used[position] = true;
                } else {
                    children.push_back(std::move(child));
                }
            }
            size_t next = 0;
            for (auto& child : children) {
                while (next < used.size() && used[next])
                    ++next;
                write_scalar<uint64_t>(child, ATTR_ORDER, H5T_NATIVE_UINT64, next);
                used[next++] = true;
            }
            for (size_t index = 0; index < all_children; ++index) {
                const auto name = object_name(group, index);
                if (name == VALUE_PAYLOAD)
                    continue;
                Id child(H5Oopen(group, name.c_str(), H5P_DEFAULT), H5Oclose,
                         "Unable to inspect a resolved node");
                if (H5Iget_type(child) == H5I_GROUP)
                    complete_resolved_order(child);
            }
        }

        void validate_source_references(const Environment& env,
                                           const std::unordered_set<std::string>& source_names) {
            auto known = source_names;
            for (const auto& source : env.applied_override_sources())
                known.insert(source.name);
            for (const auto& node : env.nodes.get_nodes()) {
                if (!node)
                    continue;
                const auto check_source = [&](const std::string& name) {
                    if (!name.empty() && !known.count(name))
                        throw Error("DIPH5 v3 node references an unarchived source: " + name);
                };
                check_source(node->line.source.name);
                if (node->override)
                    check_source(node->override_line.source.name);
                for (const auto& change : node->modification_lines)
                    check_source(change.source.name);
            }
        }
    } // namespace

    namespace {
    void write_resolved_contents(hid_t output, const Environment& env, const OutputPlan* output_plan) {
        for (const auto& node : env.nodes.get_nodes()) {
            if (node && uses_reserved_path(node->path.name))
                throw dip::IOException(
                    "Reserved DIPH5 path",
                    "The DIPL path `" + node->path.name + "` conflicts with a reserved DIPH5 manifest.",
                    "Rename the top-level `_DIPL_Sources` or `_DIPL_Trace` group before saving this environment.",
                    __FILE__,
                    __LINE__
                );
        }
        for (const auto& node : env.nodes.get_nodes()) {
            if (node && has_descendant(env, node->path.name) && uses_value_payload_child(env, node->path.name))
                throw dip::IOException(
                    "Reserved DIPH5 child path",
                    "The DIPL path `" + node->path.name + "." + std::string(VALUE_PAYLOAD) +
                        "` conflicts with the DIPH5 value-group payload.",
                    "Rename the direct child before saving this environment.",
                    __FILE__,
                    __LINE__
                );
        }
        write_trace_manifest(output, env);
        write_unit_manifest(output, env);
        write_dependency_graph(output, Inspector{env}.graph());
        if (output_plan)
            write_output_plan(output, *output_plan);
        write_hierarchy_metadata(output, env);
        for (const auto& node : env.nodes.get_nodes())
            if (node)
                write_node(output, env, *node);
        write_hierarchy_structure(output, env);
        check(H5Fflush(output, H5F_SCOPE_GLOBAL), "Unable to flush the HDF5 file");
    }
    }

    void save_staged(const Environment& env, const std::filesystem::path& file, const OutputPlan* output_plan) {
        H5Eset_auto2(H5E_DEFAULT, nullptr, nullptr);
        std::vector<ArchivedSource> model_sources;
        if (env.is_loaded_snapshot()) {
            // The archive already has the exact bytes. Reuse them when saving a copy;
            // a resolved tree alone cannot tell us what the user originally wrote.
            model_sources = env.archived_sources();
        } else {
            std::unordered_set<std::string> override_names;
            for (const auto& source : env.applied_override_sources())
                if (!source.source_name.empty())
                    override_names.insert(source.source_name);
            std::vector<std::string> source_names;
            for (const auto& name : env.sources.registration_order()) {
                const auto& source = env.sources.at(name);
                const bool synthetic_root = source.code.empty() && source.path.empty() && source.parent.name.empty();
                if (!override_names.count(name) && !synthetic_root)
                    source_names.push_back(name);
            }
            for (const auto& name : source_names) {
                const auto& source = env.sources.at(name);
                // The parser's synthetic root is not an import in the archive.
                const bool imported = std::find(source_names.begin(), source_names.end(),
                                                source.parent.name) != source_names.end();
                SourceInfo info{name, source.path, imported ? source.parent.name : "",
                                imported ? source.parent.line_number : 0, "SHA-256", snt::sha256(source.code),
                                source.metadata};
                model_sources.push_back({name, std::move(info), source.code, imported ? "import" : "model"});
            }
        }

        if (!env.is_loaded_snapshot()) {
            // Table columns are parsed with a derived source name. They still came
            // from the parent DIPL bytes, so archive that parent text under the
            // derived identity instead of leaving dangling provenance references.
            std::unordered_set<std::string> known;
            for (const auto& source : model_sources)
                known.insert(source.info.name);
            for (const auto& node : env.nodes.get_nodes()) {
                if (!node)
                    continue;
                const auto& name = node->line.source.name;
                constexpr std::string_view suffix = "_TABLE";
                if (known.count(name) || name.size() <= suffix.size() ||
                    name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0)
                    continue;
                const auto parent_name = name.substr(0, name.size() - suffix.size());
                const auto parent = std::find_if(model_sources.begin(), model_sources.end(), [&](const auto& source) {
                    return source.info.name == parent_name;
                });
                if (parent == model_sources.end())
                    continue;
                auto info = parent->info;
                info.name = name;
                model_sources.push_back({name, std::move(info), parent->content, "other"});
                known.insert(name);
            }
        }

        std::vector<std::string> model_names;
        for (const auto& source : model_sources)
            model_names.push_back(source.info.name);
        if (model_names.empty())
            throw Error("DIPH5 v3 requires at least one base-model source");
        if (model_names.size() > 100000 || env.applied_override_sources().size() > 100000)
            throw Error("DIPH5 v3 source or override count exceeds the archive limit");
        for (const auto& source : model_sources)
            if (!valid_utf8(source.content))
                throw Error("DIPH5 v3 cannot archive a source with invalid UTF-8 text");
        for (const auto& source : env.applied_override_sources())
            if (!valid_utf8(source.content))
                throw Error("DIPH5 v3 cannot archive an override with invalid UTF-8 text");
        validate_source_references(env, std::unordered_set<std::string>(model_names.begin(), model_names.end()));

        Id output(H5Fcreate(file.string().c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT), H5Fclose,
                  "Unable to create the DIPH5 v3 file");
        write_string(output, "_DIPH5_Format", "SciNumTools3 DIPH5");
        write_scalar<uint32_t>(output, "_DIPH5_Schema_Major", H5T_NATIVE_UINT32, 3);
        write_scalar<uint32_t>(output, "_DIPH5_Schema_Minor", H5T_NATIVE_UINT32, 0);
        write_scalar<uint32_t>(output, "_DIPH5_Schema_Patch", H5T_NATIVE_UINT32, 0);
        write_string(output, "_DIPH5_Record_Id", uuid());
        write_string(output, "_DIPH5_State", "prepared");
        write_string(output, "_DIPH5_Created_At", utc_now());
        write_string(output, "_DIPH5_Profile", "environment");

        Id model(H5Gcreate2(output, "/model", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                 "Unable to create v3 model group");
        Id manifest(H5Gcreate2(model, "manifest", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                    "Unable to create v3 model manifest");
        Id sources(H5Gcreate2(model, "sources", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                   "Unable to create v3 source archive");
        Id registrations(H5Gcreate2(model, "registrations", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                         "Unable to create v3 registrations");
        // This manifest contains stable source facts only. File locations and timestamps
        // are deliberately absent because moving an archive must not change its model hash.
        nlohmann::json canonical = nlohmann::json::array();
        for (size_t index = 0; index < model_sources.size(); ++index) {
            const auto& source = model_sources[index];
            const auto digest = snt::sha256(source.content);
            const auto& parent = source.info.parent_name;
            const auto& role = source.role;
            Id entry(H5Gcreate2(sources, numbered(index).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                     H5Gclose, "Unable to create v3 source entry");
            write_bytes(entry, source.content);
            write_string(entry, "source_id", source.source_id);
            write_string(entry, "name", source.info.name);
            write_string(entry, "original_path", source.info.path);
            write_string(entry, "parent_source_id", parent);
            write_scalar<uint64_t>(entry, "parent_line", H5T_NATIVE_UINT64,
                                   source.info.parent_line);
            write_string(entry, "sha256", digest);
            write_string(entry, "role", role);
            write_string(entry, "encoding", "utf-8");
            validate_metadata_utf8(source.info.metadata);
            write_metadata(entry, source.info.metadata);
            canonical.push_back({{"source_id", source.source_id}, {"name", source.info.name},
                                 {"parent_source_id", parent},
                                 {"parent_line", source.info.parent_line},
                                 {"role", role}, {"sha256", digest}});
        }
        const auto model_hash = snt::sha256(canonical.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict));
        write_string(manifest, "model_id", uuid());
        write_scalar<uint64_t>(manifest, "source_count", H5T_NATIVE_UINT64, model_names.size());
        write_string(manifest, "model_sha256", model_hash);
        write_string(manifest, "dipl_language_version", "1.1.0");
        write_string(manifest, "producer_name", "SciNumTools3");
        write_string(manifest, "producer_version", CODE_VERSION);

        Id configuration(H5Gcreate2(output, "/configuration", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                         H5Gclose, "Unable to create v3 configuration group");
        Id overrides(H5Gcreate2(configuration, "overrides", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                     H5Gclose, "Unable to create v3 override archive");
        for (size_t index = 0; index < env.applied_override_sources().size(); ++index) {
            const auto& source = env.applied_override_sources()[index];
            Id entry(H5Gcreate2(overrides, numbered(index).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                     H5Gclose, "Unable to create v3 override entry");
            write_bytes(entry, source.content);
            write_string(entry, "override_id", "override-" + numbered(index));
            write_string(entry, "name", source.name);
            write_string(entry, "origin", source.origin);
            write_string(entry, "original_path", source.path);
            write_scalar<uint64_t>(entry, "order", H5T_NATIVE_UINT64, index);
            write_string(entry, "sha256", snt::sha256(source.content));
            write_string(entry, "source_id", "");
        }
        Id resolved(H5Gcreate2(configuration, "resolved", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Gclose, "Unable to create v3 resolved hierarchy");
        Id provenance(H5Gcreate2(configuration, "provenance", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                      H5Gclose, "Unable to create v3 provenance group");
        // The resolved tree keeps the established node encoding. Its logical
        // fingerprint is calculated after the tree has been relocated below.
        write_string(provenance, "configuration_sha256", "");
        write_string(provenance, "fingerprint_status", "unavailable");
        write_string(provenance, "model_sha256", model_hash);
        write_string(provenance, "prepared_at", utc_now());
        write_string(provenance, "evaluator_name", "SciNumTools3");
        write_string(provenance, "evaluator_version", CODE_VERSION);

        // Reuse the tested v2.11 node codec in memory, then relocate its objects.
        Id staging = temporary_file();
        write_resolved_contents(staging, env, output_plan);
        // Some programmatic node trees lack sibling-order metadata. The archive
        // still needs a deterministic order so readers can validate the tree.
        for (hsize_t index = 0; index < object_count(staging); ++index) {
            const auto name = object_name(staging, index);
            const std::string from = "/" + name;
            if (name == GROUP_TRACE)
                copy_object(staging, from, registrations, "trace");
            else if (name == GROUP_UNITS)
                copy_object(staging, from, registrations, "units");
            else if (name == GROUP_DEPENDENCIES)
                copy_object(staging, from, provenance, "dependencies");
            else if (name == GROUP_OUTPUTS)
                copy_object(staging, from, configuration, "adapter_outputs");
            else
                copy_object(staging, from, resolved, name);
        }
        complete_resolved_order(resolved);
        if (const auto digest = configuration_fingerprint(output)) {
            write_string(provenance, "configuration_sha256", *digest);
            write_string(provenance, "fingerprint_status", "pass");
        }
        check(H5Fflush(output, H5F_SCOPE_GLOBAL), "Unable to flush the DIPH5 v3 file");
    }

    void save(const Environment& env, const std::filesystem::path& file, const OutputPlan* output_plan) {
        const auto temporary = file.parent_path() / (".diph5-stage-" + uuid() + ".h5");
        try {
            save_staged(env, temporary, output_plan);
#ifdef _WIN32
            if (!MoveFileExW(temporary.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                throw Error("Unable to publish staged DIPH5 file: Windows error " + std::to_string(GetLastError()));
#else
            std::error_code error;
            std::filesystem::rename(temporary, file, error);
            if (error) throw Error("Unable to publish staged DIPH5 file: " + error.message());
#endif
        } catch (...) {
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            throw;
        }
    }
    namespace {
        void load_archive(Environment& env, hid_t input, const std::filesystem::path& file) {
            if (required_utf8(input, "_DIPH5_Format") != "SciNumTools3 DIPH5" ||
                required_scalar<uint32_t>(input, "_DIPH5_Schema_Major", H5T_NATIVE_UINT32) != 3 ||
                required_scalar<uint32_t>(input, "_DIPH5_Schema_Minor", H5T_NATIVE_UINT32) != 0 ||
                required_scalar<uint32_t>(input, "_DIPH5_Schema_Patch", H5T_NATIVE_UINT32) != 0)
                throw Error("Unsupported DIPH5 v3 schema version");
            const auto profile = required_utf8(input, "_DIPH5_Profile");
            if (profile == "environment") {
                if (required_utf8(input, "_DIPH5_State") != "prepared" ||
                    has_link(input, "/execution") || has_link(input, "/results"))
                    throw Error("Invalid environment-profile DIPH5 v3 state or groups");
            } else if (profile == "experiment") {
                inspect_execution(input);
                inspect_results(input);
            } else {
                throw Error("Unsupported DIPH5 v3 profile");
            }
            if (required_utf8(input, "_DIPH5_Record_Id").empty() ||
                required_utf8(input, "_DIPH5_Created_At").empty())
                throw Error("Missing DIPH5 v3 record identity");
            Id model(H5Gopen2(input, "/model", H5P_DEFAULT), H5Gclose, "Missing v3 model group");
            Id manifest(H5Gopen2(model, "manifest", H5P_DEFAULT), H5Gclose, "Missing v3 model manifest");
            Id sources(H5Gopen2(model, "sources", H5P_DEFAULT), H5Gclose, "Missing v3 source archive");
            Id registrations(H5Gopen2(model, "registrations", H5P_DEFAULT), H5Gclose,
                             "Missing v3 registrations");
            Id configuration(H5Gopen2(input, "/configuration", H5P_DEFAULT), H5Gclose,
                             "Missing v3 configuration group");
            Id overrides(H5Gopen2(configuration, "overrides", H5P_DEFAULT), H5Gclose,
                         "Missing v3 override archive");
            Id resolved(H5Gopen2(configuration, "resolved", H5P_DEFAULT), H5Gclose,
                        "Missing v3 resolved hierarchy");
            Id provenance(H5Gopen2(configuration, "provenance", H5P_DEFAULT), H5Gclose,
                          "Missing v3 provenance group");
            // v2 readers tolerate old files without order metadata; v3 requires it.
            validate_node_order(resolved);
            const auto fingerprint_status = required_utf8(provenance, "fingerprint_status");
            const auto stored_fingerprint = required_utf8(provenance, "configuration_sha256");
            if (fingerprint_status == "pass") {
                const auto digest = configuration_fingerprint(input);
                if (!digest || *digest != stored_fingerprint)
                    throw IntegrityError("DIPH5 v3 configuration fingerprint mismatch");
            } else if (fingerprint_status != "unavailable" || !stored_fingerprint.empty()) {
                throw Error("Invalid DIPH5 v3 configuration fingerprint status");
            }

            const auto count = checked_size(object_count(sources), "v3 source count");
            if (count == 0 || count > 100000 ||
                count != required_scalar<uint64_t>(manifest, "source_count", H5T_NATIVE_UINT64))
                throw Error("Invalid DIPH5 v3 source count");
            nlohmann::json canonical = nlohmann::json::array();
            std::vector<ArchivedSource> archived;
            std::vector<SourceInfo> source_manifest;
            std::unordered_set<std::string> identities;
            std::unordered_set<std::string> names;
            for (size_t index = 0; index < count; ++index) {
                Id entry(H5Gopen2(sources, numbered(index).c_str(), H5P_DEFAULT), H5Gclose,
                         "Invalid DIPH5 v3 source numbering");
                const auto content = read_bytes(entry);
                const auto id = required_utf8(entry, "source_id");
                const auto name = required_utf8(entry, "name");
                const auto parent = required_utf8(entry, "parent_source_id");
                const auto parent_line = required_scalar<uint64_t>(entry, "parent_line", H5T_NATIVE_UINT64);
                const auto digest = required_utf8(entry, "sha256");
                const auto role = required_utf8(entry, "role");
                if (id.empty() || name.empty() || !identities.insert(id).second || !names.insert(name).second ||
                    !valid_utf8(content) || digest != snt::sha256(content) ||
                    required_utf8(entry, "encoding") != "utf-8" ||
                    (role != "model" && role != "import" && role != "units" && role != "schema" &&
                     role != "function" && role != "other") ||
                    (parent.empty() ? parent_line != 0 : parent_line == 0))
                    throw Error("Invalid DIPH5 v3 source identity, parent, encoding, or SHA-256");
                SourceInfo info{name, required_utf8(entry, "original_path"), parent,
                                checked_size(parent_line, "source parent line"), "SHA-256", digest, {}};
                read_metadata(entry, info.metadata);
                validate_metadata_utf8(info.metadata);
                archived.push_back({id, info, content, role});
                source_manifest.push_back(info);
                canonical.push_back({{"source_id", id}, {"name", name},
                                     {"parent_source_id", parent}, {"parent_line", parent_line},
                                     {"role", role}, {"sha256", digest}});
            }
            // Check parents after reading the full list; HDF5 does not promise iteration order.
            for (const auto& source : archived)
                if (!source.info.parent_name.empty() &&
                    (!identities.count(source.info.parent_name) ||
                     source.info.parent_name == source.source_id))
                    throw Error("Invalid DIPH5 v3 source parent reference");
            const auto model_hash = snt::sha256(canonical.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict));
            if (required_utf8(manifest, "model_sha256") != model_hash ||
                required_utf8(provenance, "model_sha256") != model_hash)
                throw Error("DIPH5 v3 model manifest SHA-256 mismatch");

            const auto override_count = checked_size(object_count(overrides), "v3 override count");
            if (override_count > 100000)
                throw Error("DIPH5 v3 override count exceeds the reader limit");
            for (size_t index = 0; index < override_count; ++index) {
                Id entry(H5Gopen2(overrides, numbered(index).c_str(), H5P_DEFAULT), H5Gclose,
                         "Invalid DIPH5 v3 override numbering");
                const auto content = read_bytes(entry);
                if (required_scalar<uint64_t>(entry, "order", H5T_NATIVE_UINT64) != index ||
                    !valid_utf8(content) || required_utf8(entry, "sha256") != snt::sha256(content) ||
                    required_utf8(entry, "override_id") != "override-" + numbered(index))
                    throw Error("Invalid DIPH5 v3 override order or SHA-256");
                const auto source_id = required_utf8(entry, "source_id");
                if (!source_id.empty() && !identities.count(source_id))
                    throw Error("Invalid DIPH5 v3 override source reference");
                const auto origin = required_utf8(entry, "origin");
                if (origin != "file" && origin != "inline" && origin != "generated" && origin != "api")
                    throw Error("Invalid DIPH5 v3 override origin");
                env.record_applied_override_source(
                    {required_utf8(entry, "name"), required_utf8(entry, "original_path"), content,
                     origin, source_id}
                );
                // Override text lives outside /model/sources, but its source
                // identity must still resolve when a node reports provenance.
                source_manifest.push_back({required_utf8(entry, "name"),
                                           required_utf8(entry, "original_path"), "", 0,
                                           "SHA-256", snt::sha256(content), {}});
            }
            env.set_archived_sources(std::move(archived));
            env.set_source_manifest(std::move(source_manifest));

            // The v3 layout moved the resolved-node codec but kept its encoding. Decode
            // through an in-memory file so the node and provenance checks stay in one place.
            Id staging = temporary_file();
            copy_object(registrations, "trace", staging, std::string(GROUP_TRACE));
            copy_object(registrations, "units", staging, std::string(GROUP_UNITS));
            copy_object(provenance, "dependencies", staging, std::string(GROUP_DEPENDENCIES));
            if (H5Lexists(configuration, "adapter_outputs", H5P_DEFAULT) > 0)
                copy_object(configuration, "adapter_outputs", staging, std::string(GROUP_OUTPUTS));
            for (hsize_t index = 0; index < object_count(resolved); ++index) {
                const auto name = object_name(resolved, index);
                copy_object(resolved, name, staging, name);
            }
            read_trace_manifest(env, staging);
            read_unit_manifest(env, staging);
            read_dependency_graph(env, staging);
            read_group(env, staging, true);
            read_output_plan(env, staging);
            validate_source_references(env, names);
        }
    }
    void load(Environment& env, const std::filesystem::path& file) {
        H5Eset_auto2(H5E_DEFAULT, nullptr, nullptr);
        Id input(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose, "Unable to open the HDF5 file");
        if (!has_attribute(input, "_DIPH5_Format") ||
            read_string(input, "_DIPH5_Format") != "SciNumTools3 DIPH5")
            throw Error("Unsupported DIPH5 archive: only version 3 is supported");
        load_archive(env, input, file);
    }


} // namespace snt::dip::hdf5
