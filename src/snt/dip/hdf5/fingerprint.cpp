#include "io_private.h"

#include <algorithm>
#include <optional>
#include <snt/sha256.h>

namespace snt::dip::hdf5::detail {
    namespace {
        struct Unavailable {};

        class Encoder {
          public:
            void raw(const char* bytes, size_t count) { hash_.update(bytes, count); }
            void marker(char byte) { raw(&byte, 1); }
            void number(uint64_t value) {
                char bytes[8];
                for (unsigned i = 0; i < 8; ++i) bytes[i] = static_cast<char>((value >> (8 * i)) & 0xff);
                raw(bytes, sizeof(bytes));
            }
            void text(const std::string& value) { number(value.size()); raw(value.data(), value.size()); }
            std::string finish() { return hash_.finish(); }
          private:
            snt::Sha256 hash_;
        };

        std::vector<std::string> attribute_names(hid_t object) {
            const auto count = H5Aget_num_attrs(object);
            if (count < 0 || count > 100000) throw Error("Invalid fingerprint attribute count");
            std::vector<std::string> names;
            for (int i = 0; i < count; ++i) {
                Id attr(H5Aopen_idx(object, static_cast<unsigned>(i)), H5Aclose, "Unable to inspect fingerprint attribute");
                const auto length = H5Aget_name(attr, 0, nullptr);
                if (length < 0 || length > 1024 * 1024) throw Error("Invalid fingerprint attribute name");
                std::string name(static_cast<size_t>(length) + 1, '\0');
                if (H5Aget_name(attr, name.size(), name.data()) != length) throw Error("Unable to read fingerprint attribute name");
                name.resize(static_cast<size_t>(length));
                // Timestamps are explicitly outside the configuration fingerprint.
                if (name == "created" || name == "modified") continue;
                if (name == "metadata_present") {
                    const auto values = read_strings(object, name);
                    if (std::none_of(values.begin(), values.end(), [](const auto& value) {
                            return value != "created" && value != "modified";
                        })) continue;
                }
                names.push_back(std::move(name));
            }
            std::sort(names.begin(), names.end());
            return names;
        }

        void encode_attributes(Encoder& encoder, hid_t object) {
            const auto names = attribute_names(object);
            encoder.number(names.size());
            for (const auto& name : names) {
                Id attr(H5Aopen(object, name.c_str(), H5P_DEFAULT), H5Aclose, "Unable to open fingerprint attribute");
                Id type(H5Aget_type(attr), H5Tclose, "Unable to inspect fingerprint attribute type");
                Id space(H5Aget_space(attr), H5Sclose, "Unable to inspect fingerprint attribute space");
                const auto rank = H5Sget_simple_extent_ndims(space);
                encoder.text(name);
                if (H5Tget_class(type) == H5T_STRING && rank == 0) {
                    encoder.marker('s');
                    encoder.text(read_string(object, name));
                } else if (H5Tget_class(type) == H5T_STRING && rank == 1) {
                    encoder.marker('l');
                    auto values = read_strings(object, name);
                    if (name == "metadata_present")
                        values.erase(std::remove_if(values.begin(), values.end(), [](const auto& v) {
                            return v == "created" || v == "modified";
                        }), values.end());
                    encoder.number(values.size());
                    for (const auto& value : values) encoder.text(value);
                } else if (H5Tget_class(type) == H5T_INTEGER && rank == 0 && H5Tget_sign(type) == H5T_SGN_NONE) {
                    encoder.marker('u');
                    encoder.number(required_scalar<uint64_t>(object, name, H5T_NATIVE_UINT64));
                } else {
                    throw Unavailable{};
                }
            }
        }

        hid_t numeric_type(const std::string& dtype) {
            if (dtype == "bool" || dtype == "uint8") return H5T_STD_U8LE;
            if (dtype == "char" || dtype == "int8") return H5T_STD_I8LE;
            if (dtype == "int16") return H5T_STD_I16LE;
            if (dtype == "int32") return H5T_STD_I32LE;
            if (dtype == "int64") return H5T_STD_I64LE;
            if (dtype == "uint16") return H5T_STD_U16LE;
            if (dtype == "uint32") return H5T_STD_U32LE;
            if (dtype == "uint64") return H5T_STD_U64LE;
            if (dtype == "float32") return H5T_IEEE_F32LE;
            if (dtype == "float64") return H5T_IEEE_F64LE;
            throw Unavailable{};
        }

        void encode_dataset(Encoder& encoder, hid_t dataset) {
            Id space(H5Dget_space(dataset), H5Sclose, "Unable to inspect fingerprint dataset");
            const auto dtype = required_utf8(dataset, "_DIPL_Value_Type");
            if (dtype == "float128") throw Unavailable{};
            if (H5Sget_simple_extent_type(space) == H5S_NULL) {
                encoder.marker('n');
                return;
            }
            encoder.marker('v');
            const int rank = H5Sget_simple_extent_ndims(space);
            if (rank < 0 || rank > 32) throw Error("Invalid fingerprint dataset rank");
            encoder.number(static_cast<uint64_t>(rank));
            std::vector<hsize_t> shape(static_cast<size_t>(rank));
            if (rank) check(H5Sget_simple_extent_dims(space, shape.data(), nullptr), "Invalid fingerprint dataset shape");
            size_t count = 1;
            for (auto extent : shape) {
                encoder.number(extent);
                count = checked_product(count, checked_size(extent, "fingerprint extent"), "fingerprint elements");
            }
            if (dtype == "str") {
                Id type(H5Dget_type(dataset), H5Tclose, "Unable to inspect fingerprint string");
                const auto width = H5Tget_size(type);
                const auto bytes = checked_product(count, width, "fingerprint strings");
                if (bytes > 256ULL * 1024 * 1024) throw Unavailable{};
                std::vector<char> values(bytes);
                if (count) check(H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, values.data()), "Unable to read fingerprint strings");
                for (size_t i = 0; i < count; ++i) {
                    const auto begin = values.data() + i * width;
                    const auto end = std::find(begin, begin + width, '\0');
                    if (end == begin + width) throw Error("Unterminated fingerprint string");
                    encoder.text(std::string(begin, end));
                }
            } else {
                const auto type = numeric_type(dtype);
                const auto bytes = checked_product(count, H5Tget_size(type), "fingerprint values");
                if (bytes > 256ULL * 1024 * 1024) throw Unavailable{};
                std::vector<char> values(bytes);
                if (count) check(H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, values.data()), "Unable to read fingerprint values");
                if (bytes) encoder.raw(values.data(), bytes);
            }
        }

        void encode_group(Encoder& encoder, hid_t group, size_t depth) {
            if (depth > 256) throw Error("Fingerprint hierarchy exceeds depth limit");
            auto count = checked_size(object_count(group), "fingerprint child count");
            if (count > 100000) throw Error("Fingerprint child count exceeds limit");
            std::vector<std::string> names;
            for (size_t i = 0; i < count; ++i) names.push_back(object_name(group, i));
            std::sort(names.begin(), names.end());
            encoder.number(names.size());
            for (const auto& name : names) {
                Id child(H5Oopen(group, name.c_str(), H5P_DEFAULT), H5Oclose, "Unable to open fingerprint node");
                const auto kind = H5Iget_type(child);
                encoder.marker(kind == H5I_GROUP ? 'g' : 'd');
                encoder.text(name);
                encode_attributes(encoder, child);
                if (kind == H5I_GROUP) encode_group(encoder, child, depth + 1);
                else if (kind == H5I_DATASET) encode_dataset(encoder, child);
                else throw Error("Unsupported fingerprint object type");
            }
        }
    }

    std::optional<std::string> configuration_fingerprint(hid_t archive) {
        try {
            Id manifest(H5Gopen2(archive, "/model/manifest", H5P_DEFAULT), H5Gclose, "Missing model manifest");
            Id overrides(H5Gopen2(archive, "/configuration/overrides", H5P_DEFAULT), H5Gclose, "Missing overrides");
            Id resolved(H5Gopen2(archive, "/configuration/resolved", H5P_DEFAULT), H5Gclose, "Missing resolved configuration");
            Encoder encoder;
            constexpr char domain[] = "diph5-3.0";
            encoder.raw(domain, sizeof(domain) - 1);
            encoder.text(required_utf8(manifest, "model_sha256"));
            const auto count = checked_size(object_count(overrides), "fingerprint override count");
            if (count > 100000) throw Error("Fingerprint override count exceeds limit");
            encoder.number(count);
            for (size_t index = 0; index < count; ++index) {
                Id entry(H5Gopen2(overrides, numbered(index).c_str(), H5P_DEFAULT), H5Gclose, "Missing fingerprint override");
                encoder.text(required_utf8(entry, "sha256"));
            }
            encode_group(encoder, resolved, 0);
            return encoder.finish();
        } catch (const Unavailable&) {
            return std::nullopt;
        }
    }
}
