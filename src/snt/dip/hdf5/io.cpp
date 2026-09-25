#include "io.h"

#include "schema.h"

#include <algorithm>
#include <cstdint>
#include <hdf5.h>
#include <numeric>
#include <optional>
#include <set>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/val/values.h>
#include <string>
#include <vector>

namespace snt::dip::hdf5 {
    namespace {
        using namespace schema;

        class Id {
          public:
            using Closer = herr_t (*)(hid_t);

            Id() = default;
            Id(hid_t value, Closer closer, const char* operation) : value_(value), closer_(closer) {
                if (value_ < 0)
                    throw Error(operation);
            }
            Id(const Id&) = delete;
            Id& operator=(const Id&) = delete;
            Id(Id&& other) noexcept : value_(other.value_), closer_(other.closer_) { other.value_ = H5I_INVALID_HID; }
            Id& operator=(Id&& other) noexcept {
                if (this != &other) {
                    close();
                    value_ = other.value_;
                    closer_ = other.closer_;
                    other.value_ = H5I_INVALID_HID;
                }
                return *this;
            }
            ~Id() { close(); }

            operator hid_t() const { return value_; }

          private:
            void close() noexcept {
                if (value_ >= 0 && closer_)
                    closer_(value_);
                value_ = H5I_INVALID_HID;
            }

            hid_t value_ = H5I_INVALID_HID;
            Closer closer_ = nullptr;
        };

        void check(herr_t status, const char* operation) {
            if (status < 0)
                throw Error(operation);
        }

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

        Id string_type(size_t size) {
            Id type(H5Tcopy(H5T_C_S1), H5Tclose, "Unable to create an HDF5 string type");
            check(H5Tset_size(type, std::max<size_t>(size + 1, 1)), "Unable to size an HDF5 string type");
            check(H5Tset_cset(type, H5T_CSET_UTF8), "Unable to set HDF5 UTF-8 encoding");
            check(H5Tset_strpad(type, H5T_STR_NULLTERM), "Unable to set HDF5 string padding");
            return type;
        }

        bool has_attribute(hid_t object, std::string_view name) {
            const htri_t result = H5Aexists(object, std::string(name).c_str());
            if (result < 0)
                throw Error("Unable to inspect an HDF5 attribute");
            return result > 0;
        }

        void remove_attribute(hid_t object, std::string_view name) {
            if (has_attribute(object, name))
                check(H5Adelete(object, std::string(name).c_str()), "Unable to replace an HDF5 attribute");
        }

        void write_string(hid_t object, std::string_view name, const std::string& value) {
            remove_attribute(object, name);
            Id type = string_type(value.size());
            Id space(H5Screate(H5S_SCALAR), H5Sclose, "Unable to create an HDF5 attribute dataspace");
            Id attribute(
                H5Acreate2(object, std::string(name).c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT),
                H5Aclose,
                "Unable to create an HDF5 string attribute"
            );
            check(H5Awrite(attribute, type, value.c_str()), "Unable to write an HDF5 string attribute");
        }

        std::string read_string(hid_t object, std::string_view name, const std::string& fallback = {}) {
            if (!has_attribute(object, name))
                return fallback;
            Id attribute(
                H5Aopen(object, std::string(name).c_str(), H5P_DEFAULT), H5Aclose, "Unable to open an HDF5 attribute"
            );
            Id type(H5Aget_type(attribute), H5Tclose, "Unable to inspect an HDF5 string attribute");
            const size_t width = H5Tget_size(type);
            if (width == 0)
                throw Error("Invalid HDF5 string attribute");
            std::vector<char> buffer(width + 1, '\0');
            check(H5Aread(attribute, type, buffer.data()), "Unable to read an HDF5 string attribute");
            return buffer.data();
        }

        template <typename T> void write_scalar(hid_t object, std::string_view name, hid_t type, const T& value) {
            remove_attribute(object, name);
            Id space(H5Screate(H5S_SCALAR), H5Sclose, "Unable to create an HDF5 attribute dataspace");
            Id attribute(
                H5Acreate2(object, std::string(name).c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT),
                H5Aclose,
                "Unable to create an HDF5 scalar attribute"
            );
            check(H5Awrite(attribute, type, &value), "Unable to write an HDF5 scalar attribute");
        }

        template <typename T> T read_scalar(hid_t object, std::string_view name, hid_t type, T fallback = {}) {
            if (!has_attribute(object, name))
                return fallback;
            Id attribute(
                H5Aopen(object, std::string(name).c_str(), H5P_DEFAULT), H5Aclose, "Unable to open an HDF5 attribute"
            );
            T value{};
            check(H5Aread(attribute, type, &value), "Unable to read an HDF5 scalar attribute");
            return value;
        }

        void write_strings(hid_t object, std::string_view name, const std::vector<std::string>& values) {
            if (values.empty())
                return;
            size_t width = 1;
            for (const auto& value : values)
                width = std::max(width, value.size() + 1);
            std::vector<char> buffer(values.size() * width, '\0');
            for (size_t i = 0; i < values.size(); ++i)
                std::copy(values[i].begin(), values[i].end(), buffer.begin() + static_cast<ptrdiff_t>(i * width));
            hsize_t count = values.size();
            Id space(H5Screate_simple(1, &count, nullptr), H5Sclose, "Unable to create an HDF5 array dataspace");
            Id type(H5Tcopy(H5T_C_S1), H5Tclose, "Unable to create an HDF5 string type");
            check(H5Tset_size(type, width), "Unable to size an HDF5 string type");
            check(H5Tset_cset(type, H5T_CSET_UTF8), "Unable to set HDF5 UTF-8 encoding");
            check(H5Tset_strpad(type, H5T_STR_NULLTERM), "Unable to set HDF5 string padding");
            remove_attribute(object, name);
            Id attribute(
                H5Acreate2(object, std::string(name).c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT),
                H5Aclose,
                "Unable to create an HDF5 string-array attribute"
            );
            check(H5Awrite(attribute, type, buffer.data()), "Unable to write an HDF5 string-array attribute");
        }

        std::vector<std::string> read_strings(hid_t object, std::string_view name) {
            if (!has_attribute(object, name))
                return {};
            Id attribute(
                H5Aopen(object, std::string(name).c_str(), H5P_DEFAULT), H5Aclose, "Unable to open an HDF5 attribute"
            );
            Id space(H5Aget_space(attribute), H5Sclose, "Unable to inspect an HDF5 attribute dataspace");
            hsize_t count = 0;
            check(H5Sget_simple_extent_dims(space, &count, nullptr), "Unable to inspect an HDF5 attribute shape");
            Id type(H5Aget_type(attribute), H5Tclose, "Unable to inspect an HDF5 string attribute");
            const size_t width = H5Tget_size(type);
            std::vector<char> buffer(static_cast<size_t>(count) * width, '\0');
            check(H5Aread(attribute, type, buffer.data()), "Unable to read an HDF5 string-array attribute");
            std::vector<std::string> values;
            values.reserve(count);
            for (size_t i = 0; i < count; ++i)
                values.emplace_back(buffer.data() + i * width);
            return values;
        }

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
            return NodeDtype::None;
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
                    hdf5_path += '/' + item;
                    dipl_path += '[' + item + ']';
                    Id item_group =
                        ensure_group(file, hdf5_path, collection.kind == Path::Kind::List ? "list_item" : "map_item");
                    write_string(item_group, ATTR_PATH, dipl_path);
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
                const auto values = value->get_values();
                size_t width = 1;
                for (const auto& item : values)
                    width = std::max(width, item.size() + 1);
                Id type(H5Tcopy(H5T_C_S1), H5Tclose, "Unable to create an HDF5 string type");
                check(H5Tset_size(type, width), "Unable to size an HDF5 string type");
                check(H5Tset_cset(type, H5T_CSET_UTF8), "Unable to set HDF5 UTF-8 encoding");
                check(H5Tset_strpad(type, H5T_STR_NULLTERM), "Unable to set HDF5 string padding");
                std::vector<char> buffer(values.size() * width, '\0');
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
#define SNT_WRITE_METADATA(NAME)                                                                                       \
    if (!metadata.NAME.empty())                                                                                        \
    write_string(object, #NAME, metadata.NAME)
            SNT_WRITE_METADATA(description);
            SNT_WRITE_METADATA(authors);
            SNT_WRITE_METADATA(title);
            SNT_WRITE_METADATA(journal);
            SNT_WRITE_METADATA(year);
            SNT_WRITE_METADATA(volume);
            SNT_WRITE_METADATA(issue);
            SNT_WRITE_METADATA(pages);
            SNT_WRITE_METADATA(doi);
            SNT_WRITE_METADATA(url);
            SNT_WRITE_METADATA(version);
            SNT_WRITE_METADATA(created);
            SNT_WRITE_METADATA(modified);
            SNT_WRITE_METADATA(license);
            SNT_WRITE_METADATA(rationale);
            SNT_WRITE_METADATA(recommended_range);
            SNT_WRITE_METADATA(performance_impact);
            SNT_WRITE_METADATA(scientific_impact);
            SNT_WRITE_METADATA(deprecated);
            SNT_WRITE_METADATA(replacement);
            SNT_WRITE_METADATA(since);
            SNT_WRITE_METADATA(category);
            SNT_WRITE_METADATA(visibility);
#undef SNT_WRITE_METADATA
#define SNT_WRITE_METADATA_LIST(NAME)                                                                                  \
    if (!metadata.NAME.empty())                                                                                        \
    write_strings(object, #NAME, metadata.NAME)
            SNT_WRITE_METADATA_LIST(native);
            SNT_WRITE_METADATA_LIST(requires);
            SNT_WRITE_METADATA_LIST(conflicts);
            SNT_WRITE_METADATA_LIST(implies);
            SNT_WRITE_METADATA_LIST(see);
            SNT_WRITE_METADATA_LIST(example);
#undef SNT_WRITE_METADATA_LIST
        }

        void write_value_dataset(hid_t file, const std::string& path, const ValueNode& node) {
            write_value(file, path, node);
            Id dataset(H5Dopen2(file, path.c_str(), H5P_DEFAULT), H5Dclose, "Unable to open an HDF5 dataset");
            write_string(dataset, ATTR_KIND, "value");
            write_string(dataset, ATTR_PATH, node.path.name);
            write_string(dataset, ATTR_NODE_TYPE, node_type_name(node.dtype));
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
            option_values.reserve(node.options.size());
            option_units.reserve(node.options.size());
            for (const auto& option : node.options) {
                option_values.push_back(option.value_raw);
                option_units.push_back(option.units_raw);
            }
            write_strings(dataset, ATTR_OPTIONS, option_values);
            write_strings(dataset, ATTR_OPTION_UNITS, option_units);
            write_strings(dataset, ATTR_SCHEMAS, node.schemas);
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
            if (!has_descendant(env, node.path.name)) {
                write_value_dataset(file, path, node);
                return;
            }
            Id group = ensure_group(file, path, std::string(KIND_VALUE_GROUP));
            write_string(group, ATTR_PATH, node.path.name);
            write_value_dataset(group, std::string(VALUE_PAYLOAD), node);
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

        size_t element_count(const std::vector<hsize_t>& dims) {
            return dims.empty() ? 1 : std::accumulate(dims.begin(), dims.end(), size_t{1}, std::multiplies<>());
        }

        template <typename Disk, typename Stored>
        val::BaseValue::PointerType read_numeric(
            hid_t dataset, hid_t type, core::DataType dtype, const val::Array::ShapeType& shape
        ) {
            std::vector<Disk> disk(element_count(std::vector<hsize_t>(shape.begin(), shape.end())));
            check(H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, disk.data()), "Unable to read an HDF5 dataset");
            std::vector<Stored> stored(disk.begin(), disk.end());
            return std::make_unique<val::ArrayValue<Stored>>(stored, shape, dtype);
        }

        val::BaseValue::PointerType read_value(hid_t dataset, core::DataType dtype) {
            Id space(H5Dget_space(dataset), H5Sclose, "Unable to inspect an HDF5 dataset dataspace");
            if (H5Sget_simple_extent_type(space) == H5S_NULL)
                return nullptr;
            const auto disk_dims = dimensions(dataset);
            val::Array::ShapeType shape;
            if (disk_dims.empty())
                shape = {1};
            else
                shape.assign(disk_dims.begin(), disk_dims.end());
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
                const size_t count = element_count(std::vector<hsize_t>(shape.begin(), shape.end()));
                std::vector<char> buffer(count * width, '\0');
                check(
                    H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()),
                    "Unable to read an HDF5 string dataset"
                );
                std::vector<std::string> values;
                values.reserve(count);
                for (size_t i = 0; i < count; ++i)
                    values.emplace_back(buffer.data() + i * width);
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
#define SNT_READ_METADATA(NAME) metadata.NAME = read_string(object, #NAME)
            SNT_READ_METADATA(description);
            SNT_READ_METADATA(authors);
            SNT_READ_METADATA(title);
            SNT_READ_METADATA(journal);
            SNT_READ_METADATA(year);
            SNT_READ_METADATA(volume);
            SNT_READ_METADATA(issue);
            SNT_READ_METADATA(pages);
            SNT_READ_METADATA(doi);
            SNT_READ_METADATA(url);
            SNT_READ_METADATA(version);
            SNT_READ_METADATA(created);
            SNT_READ_METADATA(modified);
            SNT_READ_METADATA(license);
            SNT_READ_METADATA(rationale);
            SNT_READ_METADATA(recommended_range);
            SNT_READ_METADATA(performance_impact);
            SNT_READ_METADATA(scientific_impact);
            SNT_READ_METADATA(deprecated);
            SNT_READ_METADATA(replacement);
            SNT_READ_METADATA(since);
            SNT_READ_METADATA(category);
            SNT_READ_METADATA(visibility);
#undef SNT_READ_METADATA
#define SNT_READ_METADATA_LIST(NAME) metadata.NAME = read_strings(object, #NAME)
            SNT_READ_METADATA_LIST(native);
            SNT_READ_METADATA_LIST(requires);
            SNT_READ_METADATA_LIST(conflicts);
            SNT_READ_METADATA_LIST(implies);
            SNT_READ_METADATA_LIST(see);
            SNT_READ_METADATA_LIST(example);
#undef SNT_READ_METADATA_LIST
        }

        void read_dataset(Environment& env, hid_t dataset) {
            if (read_string(dataset, ATTR_KIND) != "value")
                return;
            const std::string path = read_string(dataset, ATTR_PATH);
            const core::DataType value_dtype = data_type(read_string(dataset, ATTR_VALUE_TYPE));
            val::BaseValue::PointerType value = read_value(dataset, value_dtype);
            std::optional<puq::Quantity> units;
            if (has_attribute(dataset, "units"))
                units.emplace(read_string(dataset, "units"));

            ValueNode::PointerType node;
            const NodeDtype dtype = node_type(read_string(dataset, ATTR_NODE_TYPE));
            if (value)
                node = std::make_shared<PersistedValueNode>(Path(path), std::move(value), dtype, units);
            else
                node = std::make_shared<PersistedValueNode>(Path(path), value_dtype, dtype);
            node->constant = read_scalar<uint8_t>(dataset, ATTR_CONSTANT, H5T_NATIVE_UINT8) != 0;
            node->condition = read_string(dataset, ATTR_CONDITION);
            node->format = read_string(dataset, ATTR_FORMAT_SPEC);
            node->tags = read_strings(dataset, ATTR_TAGS);
            const auto option_values = read_strings(dataset, ATTR_OPTIONS);
            const auto option_units = read_strings(dataset, ATTR_OPTION_UNITS);
            for (size_t i = 0; i < option_values.size(); ++i) {
                const std::string units_value = i < option_units.size() ? option_units[i] : "";
                node->options.push_back({option_value(value_dtype, option_values[i]), option_values[i], units_value});
            }
            node->schemas = read_strings(dataset, ATTR_SCHEMAS);
            if (!node->schemas.empty())
                throw dip::IOException(
                    "Invalid value-node schema",
                    "The persisted value node `" + path + "` has applied schemas.",
                    "Use a DIPH5 file where schemas are applied only to groups or collection items.",
                    __FILE__,
                    __LINE__
                );
            node->value_origin =
                static_cast<ValueOrigin>(read_scalar<uint64_t>(dataset, ATTR_VALUE_ORIGIN, H5T_NATIVE_UINT64));
            node->line.source.name = read_string(dataset, ATTR_SOURCE);
            node->line.source.line_number = read_scalar<uint64_t>(dataset, ATTR_SOURCE_LINE, H5T_NATIVE_UINT64);
            node->line.code = read_string(dataset, ATTR_SOURCE_CODE);
            read_metadata(dataset, node->metadata);
            env.nodes.push_back(node);
            if (!env.hierarchy.has_collection(path))
                env.hierarchy.set_collection(path, Path::Kind::Group, node->schemas);
        }

        hsize_t object_count(hid_t group) {
            hsize_t count = 0;
            check(H5Gget_num_objs(group, &count), "Unable to count objects in an HDF5 group");
            return count;
        }

        std::string object_name(hid_t group, hsize_t index) {
            const ssize_t size = H5Gget_objname_by_idx(group, index, nullptr, 0);
            if (size < 0)
                throw Error("Unable to inspect an HDF5 object name");
            std::vector<char> name(static_cast<size_t>(size) + 1, '\0');
            if (H5Gget_objname_by_idx(group, index, name.data(), name.size()) < 0)
                throw Error("Unable to read an HDF5 object name");
            return name.data();
        }

        void read_source_manifest(Environment& env, hid_t file) {
            const std::string manifest_path = "/" + std::string(GROUP_SOURCES);
            const htri_t exists = H5Lexists(file, manifest_path.c_str(), H5P_DEFAULT);
            if (exists < 0)
                throw Error("Unable to inspect the HDF5 source manifest");
            if (exists == 0)
                throw dip::IOException(
                    "Invalid HDF5 source manifest",
                    "A DIPH5 version 2 file does not contain its required source manifest.",
                    "Use a complete DIPH5 file written by a compatible SciNumTools3 version.",
                    __FILE__,
                    __LINE__
                );

            Id manifest(
                H5Gopen2(file, manifest_path.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open the HDF5 source manifest"
            );
            std::vector<SourceInfo> sources;
            const hsize_t count = object_count(manifest);
            sources.reserve(count);
            for (hsize_t index = 0; index < count; ++index) {
                const std::string entry_name = object_name(manifest, index);
                if (H5Gget_objtype_by_idx(manifest, index) != H5G_GROUP)
                    throw dip::IOException(
                        "Invalid HDF5 source manifest",
                        "A source-manifest entry is not an HDF5 group.",
                        "Use a complete DIPH5 file written by a compatible SciNumTools3 version.",
                        __FILE__,
                        __LINE__
                    );
                Id entry(
                    H5Gopen2(manifest, entry_name.c_str(), H5P_DEFAULT),
                    H5Gclose,
                    "Unable to open an HDF5 source-manifest entry"
                );
                SourceInfo source{
                    read_string(entry, ATTR_SOURCE_NAME),
                    read_string(entry, ATTR_SOURCE_PATH),
                    read_string(entry, ATTR_SOURCE_PARENT),
                    static_cast<size_t>(read_scalar<uint64_t>(entry, ATTR_SOURCE_PARENT_LINE, H5T_NATIVE_UINT64)),
                    read_string(entry, ATTR_SOURCE_HASH_ALGORITHM),
                    read_string(entry, ATTR_SOURCE_HASH),
                };
                if (source.name.empty() || source.hash_algorithm.empty() || source.hash.empty())
                    throw dip::IOException(
                        "Invalid HDF5 source manifest",
                        "A source-manifest entry is missing its identity or content fingerprint.",
                        "Use a complete DIPH5 file written by a compatible SciNumTools3 version.",
                        __FILE__,
                        __LINE__
                    );
                sources.push_back(std::move(source));
            }
            env.set_source_manifest(std::move(sources));
        }

        void read_trace_manifest(Environment& env, hid_t file) {
            const std::string manifest_path = "/" + std::string(GROUP_TRACE);
            const htri_t exists = H5Lexists(file, manifest_path.c_str(), H5P_DEFAULT);
            if (exists < 0)
                throw Error("Unable to inspect the HDF5 trace manifest");
            if (exists == 0)
                throw dip::IOException(
                    "Invalid HDF5 trace manifest",
                    "A DIPH5 version 2.1 file does not contain its required trace manifest.",
                    "Use a complete DIPH5 file written by a compatible SciNumTools3 version.",
                    __FILE__,
                    __LINE__
                );

            Id manifest(
                H5Gopen2(file, manifest_path.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open the HDF5 trace manifest"
            );
            std::vector<TraceInfo> traces;
            const hsize_t count = object_count(manifest);
            traces.reserve(count);
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
                traces.push_back(std::move(trace));
            }
            env.set_trace_manifest(std::move(traces));
        }

        void read_unit_manifest(Environment& env, hid_t file) {
            const std::string manifest_path = "/" + std::string(GROUP_UNITS);
            const htri_t exists = H5Lexists(file, manifest_path.c_str(), H5P_DEFAULT);
            if (exists < 0)
                throw Error("Unable to inspect the HDF5 unit manifest");
            if (exists == 0)
                throw dip::IOException(
                    "Invalid HDF5 unit manifest",
                    "A DIPH5 version 2.3 file does not contain its required custom-unit manifest.",
                    "Use a complete DIPH5 2.3 file written by a compatible SciNumTools3 version.",
                    __FILE__,
                    __LINE__
                );
            Id manifest(
                H5Gopen2(file, manifest_path.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open the HDF5 unit manifest"
            );
            struct Record { std::string name; std::string definition; std::string id; uint64_t order; };
            std::vector<Record> records;
            std::set<std::string> names;
            std::set<std::string> ids;
            std::set<uint64_t> orders;
            const hsize_t count = object_count(manifest);
            records.reserve(count);
            for (hsize_t index = 0; index < count; ++index) {
                const std::string entry_name = object_name(manifest, index);
                if (H5Gget_objtype_by_idx(manifest, index) != H5G_GROUP)
                    throw dip::IOException("Invalid HDF5 unit manifest", "A unit-manifest entry is not an HDF5 group.", "Use a complete DIPH5 2.3 file.", __FILE__, __LINE__);
                Id entry(H5Gopen2(manifest, entry_name.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open an HDF5 unit-manifest entry");
                Record record{read_string(entry, ATTR_UNIT_NAME), read_string(entry, ATTR_UNIT_DEFINITION),
                              read_string(entry, ATTR_TRACE_ID), read_scalar<uint64_t>(entry, ATTR_UNIT_ORDER, H5T_NATIVE_UINT64)};
                if (record.name.empty() || record.definition.empty() || record.id.empty() || !names.insert(record.name).second ||
                    !ids.insert(record.id).second || !orders.insert(record.order).second)
                    throw dip::IOException("Invalid HDF5 unit manifest", "A unit-manifest entry has missing or duplicate identity data.", "Use a complete DIPH5 2.3 file.", __FILE__, __LINE__);
                const auto traces = env.get_trace_manifest();
                const auto trace = std::find_if(traces.begin(), traces.end(), [&](const TraceInfo& item) { return item.id == record.id; });
                if (trace == traces.end() || trace->name != record.name || trace->kind != "unit")
                    throw dip::IOException("Invalid HDF5 unit manifest", "A unit-manifest trace identifier does not match the trace manifest.", "Use a complete DIPH5 2.3 file.", __FILE__, __LINE__);
                records.push_back(std::move(record));
            }
            std::sort(records.begin(), records.end(), [](const Record& left, const Record& right) { return left.order < right.order; });
            for (const auto& record : records) {
                try {
                    env.units.append(record.name, EnvUnit{record.name, record.definition, 0, record.id, static_cast<size_t>(record.order)});
                } catch (const std::exception& error) {
                    throw dip::IOException("Invalid HDF5 unit manifest", "Unable to register custom unit `" + record.name + "`: " + error.what(), "Ensure unit definitions are valid and ordered by dependency.", __FILE__, __LINE__);
                }
            }
        }

        void read_group(Environment& env, hid_t group, bool root = false, bool skip_value_payload = false) {
            const hsize_t count = object_count(group);
            for (hsize_t i = 0; i < count; ++i) {
                const std::string name = object_name(group, i);
                if (root && (name == std::string(GROUP_SOURCES) || name == std::string(GROUP_TRACE) || name == std::string(GROUP_UNITS)))
                    continue;
                if (skip_value_payload && name == VALUE_PAYLOAD)
                    continue;
                const int type = H5Gget_objtype_by_idx(group, i);
                if (type == H5G_GROUP) {
                    Id child(H5Gopen2(group, name.c_str(), H5P_DEFAULT), H5Gclose, "Unable to open an HDF5 group");
                    const std::string kind = read_string(child, ATTR_KIND);
                    const std::string path = read_string(child, ATTR_PATH);
                    if (kind == KIND_VALUE_GROUP) {
                        const htri_t payload_exists = H5Lexists(child, std::string(VALUE_PAYLOAD).c_str(), H5P_DEFAULT);
                        if (payload_exists <= 0)
                            throw dip::IOException(
                                "Invalid DIPH5 value group",
                                "The value group at `" + path + "` does not contain its `_DIPL_Value` payload.",
                                "Use a complete DIPH5 2.2 file written by a compatible SciNumTools3 version.",
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
                                "Use a complete DIPH5 2.2 file written by a compatible SciNumTools3 version.",
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
                        env.hierarchy.set_collection(path, kind == "group" ? Path::Kind::Group : Path::Kind::Item, {});
                    if (!path.empty() && (kind == "map" || kind == "list") && !env.hierarchy.has_collection(path)) {
                        std::vector<std::string> items;
                        const hsize_t child_count = object_count(child);
                        for (hsize_t j = 0; j < child_count; ++j) {
                            if (H5Gget_objtype_by_idx(child, j) != H5G_GROUP)
                                continue;
                            const std::string item_name = object_name(child, j);
                            Id item(
                                H5Gopen2(child, item_name.c_str(), H5P_DEFAULT),
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
                            path, kind == "map" ? Path::Kind::Map : Path::Kind::List, {}, items
                        );
                    }
                    read_group(env, child);
                } else if (type == H5G_DATASET) {
                    Id dataset(H5Dopen2(group, name.c_str(), H5P_DEFAULT), H5Dclose, "Unable to open an HDF5 dataset");
                    read_dataset(env, dataset);
                }
            }
        }

        void write_source_manifest(hid_t file, const Environment& env) {
            const std::string manifest_path = "/" + std::string(GROUP_SOURCES);
            Id manifest(
                H5Gcreate2(file, manifest_path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                H5Gclose,
                "Unable to create the HDF5 source manifest"
            );
            const auto sources = env.get_source_manifest();
            for (size_t index = 0; index < sources.size(); ++index) {
                const SourceInfo& source = sources[index];
                const std::string entry_name = std::to_string(index);
                Id entry(
                    H5Gcreate2(manifest, entry_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Gclose,
                    "Unable to create an HDF5 source-manifest entry"
                );
                write_string(entry, ATTR_SOURCE_NAME, source.name);
                write_string(entry, ATTR_SOURCE_PATH, source.path);
                write_string(entry, ATTR_SOURCE_PARENT, source.parent_name);
                write_scalar<uint64_t>(entry, ATTR_SOURCE_PARENT_LINE, H5T_NATIVE_UINT64, source.parent_line);
                write_string(entry, ATTR_SOURCE_HASH_ALGORITHM, source.hash_algorithm);
                write_string(entry, ATTR_SOURCE_HASH, source.hash);
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
            }
        }

        void write_unit_manifest(hid_t file, const Environment& env) {
            const std::string manifest_path = "/" + std::string(GROUP_UNITS);
            Id manifest(H5Gcreate2(file, manifest_path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose, "Unable to create the HDF5 unit manifest");
            std::vector<const EnvUnit*> units;
            units.reserve(env.units.entries().size());
            for (const auto& entry : env.units.entries())
                units.push_back(&entry.second);
            std::sort(units.begin(), units.end(), [](const EnvUnit* left, const EnvUnit* right) { return left->registration_order < right->registration_order; });
            for (size_t index = 0; index < units.size(); ++index) {
                const EnvUnit& unit = *units[index];
                Id entry(H5Gcreate2(manifest, std::to_string(index).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose, "Unable to create an HDF5 unit-manifest entry");
                write_string(entry, ATTR_UNIT_NAME, unit.name);
                write_string(entry, ATTR_UNIT_DEFINITION, unit.definition);
                write_scalar<uint64_t>(entry, ATTR_UNIT_ORDER, H5T_NATIVE_UINT64, unit.registration_order);
                write_string(entry, ATTR_TRACE_ID, unit.id);
            }
        }

        bool uses_reserved_path(const std::string& path) {
            for (const auto name : {std::string(GROUP_SOURCES), std::string(GROUP_TRACE), std::string(GROUP_UNITS)}) {
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
    } // namespace

    void save(const Environment& env, const std::filesystem::path& file) {
        H5Eset_auto2(H5E_DEFAULT, nullptr, nullptr);
        Id output(
            H5Fcreate(file.string().c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT),
            H5Fclose,
            "Unable to create the HDF5 file"
        );
        write_string(output, ATTR_FORMAT, std::string(FORMAT));
        write_scalar<uint64_t>(output, ATTR_VERSION, H5T_NATIVE_UINT64, VERSION);
        write_scalar<uint64_t>(output, ATTR_VERSION_MINOR, H5T_NATIVE_UINT64, VERSION_MINOR);
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
        write_source_manifest(output, env);
        write_trace_manifest(output, env);
        write_unit_manifest(output, env);
        for (const auto& node : env.nodes.get_nodes())
            if (node)
                write_node(output, env, *node);
        check(H5Fflush(output, H5F_SCOPE_GLOBAL), "Unable to flush the HDF5 file");
    }

    void load(Environment& env, const std::filesystem::path& file) {
        H5Eset_auto2(H5E_DEFAULT, nullptr, nullptr);
        Id input(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose, "Unable to open the HDF5 file");
        if (read_string(input, ATTR_FORMAT) != FORMAT)
            throw dip::IOException(
                "Invalid HDF5 environment",
                "The file does not identify itself as a SciNumTools3 environment.",
                "Load a file created by Environment::save().",
                __FILE__,
                __LINE__
            );
        const uint64_t version = read_scalar<uint64_t>(input, ATTR_VERSION, H5T_NATIVE_UINT64);
        if (version < FIRST_SUPPORTED_VERSION || version > VERSION)
            throw dip::IOException(
                "Unsupported HDF5 environment version",
                "The file schema version is not supported.",
                "Use a compatible SciNumTools3 version.",
                __FILE__,
                __LINE__
            );
        const uint64_t minor_version = has_attribute(input, ATTR_VERSION_MINOR)
                                           ? read_scalar<uint64_t>(input, ATTR_VERSION_MINOR, H5T_NATIVE_UINT64)
                                           : 0;
        if (version == VERSION && minor_version > VERSION_MINOR)
            throw dip::IOException(
                "Unsupported HDF5 environment version",
                "The file schema minor version is newer than this SciNumTools3 version supports.",
                "Use a compatible SciNumTools3 version.",
                __FILE__,
                __LINE__
            );
        if (version >= 2)
            read_source_manifest(env, input);
        if (version == VERSION && minor_version >= 1)
            read_trace_manifest(env, input);
        if (version == VERSION && minor_version >= 3)
            read_unit_manifest(env, input);
        read_group(env, input, true);
    }

} // namespace snt::dip::hdf5
