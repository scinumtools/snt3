#include "io_private.h"
#include "../input/utf8.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <limits>
#include <random>
#include <snt/sha256.h>
#include <sstream>

namespace snt::dip::hdf5::detail {
        void check(herr_t status, const char* operation) {
            if (status < 0)
                throw Error(operation);
        }

        size_t checked_size(uint64_t value, const char* field) {
            if (value > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
                throw Error(std::string("DIPH5 ") + field + " exceeds SIZE_MAX");
            return static_cast<size_t>(value);
        }

        size_t checked_product(size_t count, size_t width, const char* field) {
            if (width != 0 && count > std::numeric_limits<size_t>::max() / width)
                throw Error(std::string("DIPH5 ") + field + " exceeds SIZE_MAX");
            return count * width;
        }

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
            if (!valid_utf8(value)) throw Error("Invalid UTF-8 in DIPH5 string attribute: " + std::string(name));
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

        std::string read_string(hid_t object, std::string_view name, const std::string& fallback) {
            if (!has_attribute(object, name))
                return fallback;
            Id attribute(
                H5Aopen(object, std::string(name).c_str(), H5P_DEFAULT), H5Aclose, "Unable to open an HDF5 attribute"
            );
            Id type(H5Aget_type(attribute), H5Tclose, "Unable to inspect an HDF5 string attribute");
            Id space(H5Aget_space(attribute), H5Sclose, "Unable to inspect an HDF5 string attribute shape");
            if (H5Tget_class(type) != H5T_STRING || H5Tis_variable_str(type) != 0 ||
                H5Sget_simple_extent_ndims(space) != 0)
                throw Error("Invalid HDF5 scalar string attribute");
            const size_t width = H5Tget_size(type);
            if (width == 0 || width > 1024 * 1024)
                throw Error("Invalid HDF5 string attribute");
            std::vector<char> buffer(width + 1, '\0');
            check(H5Aread(attribute, type, buffer.data()), "Unable to read an HDF5 string attribute");
            const std::string value(buffer.data());
            if (!valid_utf8(value)) throw Error("Invalid UTF-8 in DIPH5 string attribute: " + std::string(name));
            return value;
        }

        void write_strings(hid_t object, std::string_view name, const std::vector<std::string>& values) {
            if (values.empty())
                return;
            size_t width = 1;
            for (const auto& value : values) {
                if (!valid_utf8(value)) throw Error("Invalid UTF-8 in DIPH5 string-array attribute: " + std::string(name));
                width = std::max(width, value.size() + 1);
            }
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
            if (H5Sget_simple_extent_ndims(space) != 1)
                throw Error("Invalid HDF5 string-array attribute shape");
            hsize_t count = 0;
            check(H5Sget_simple_extent_dims(space, &count, nullptr), "Unable to inspect an HDF5 attribute shape");
            Id type(H5Aget_type(attribute), H5Tclose, "Unable to inspect an HDF5 string attribute");
            if (H5Tget_class(type) != H5T_STRING || H5Tis_variable_str(type) != 0)
                throw Error("Invalid HDF5 string-array attribute type");
            const size_t width = H5Tget_size(type);
            if (width == 0 || width > 1024 * 1024)
                throw Error("Invalid HDF5 string-array attribute");
            const size_t item_count = checked_size(count, "string-array count");
            if (item_count == 0)
                return {};
            const auto bytes = checked_product(item_count, width, "string-array size");
            if (bytes > 256ULL * 1024 * 1024)
                throw Error("HDF5 string-array attribute exceeds the reader limit");
            std::vector<char> buffer(bytes, '\0');
            check(H5Aread(attribute, type, buffer.data()), "Unable to read an HDF5 string-array attribute");
            std::vector<std::string> values;
            values.reserve(item_count);
            for (size_t i = 0; i < item_count; ++i) {
                const char* begin = buffer.data() + i * width;
                const char* end = std::find(begin, begin + width, '\0');
                values.emplace_back(begin, end);
                if (!valid_utf8(values.back()))
                    throw Error("Invalid UTF-8 in DIPH5 string-array attribute: " + std::string(name));
            }
            return values;
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
            const size_t width = checked_size(static_cast<uint64_t>(size), "object name length");
            if (width == std::numeric_limits<size_t>::max())
                throw Error("DIPH5 object name length exceeds SIZE_MAX");
            std::vector<char> name(width + 1, '\0');
            if (H5Gget_objname_by_idx(group, index, name.data(), name.size()) < 0)
                throw Error("Unable to read an HDF5 object name");
            return name.data();
        }

        std::string numbered(size_t index) {
            std::ostringstream out;
            out << std::setw(6) << std::setfill('0') << index;
            return out.str();
        }

        std::string utc_now() {
            const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm utc{};
#ifdef _WIN32
            gmtime_s(&utc, &now);
#else
            gmtime_r(&now, &utc);
#endif
            std::ostringstream out;
            out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
            return out.str();
        }

        std::string uuid() {
            std::random_device random;
            std::array<unsigned char, 16> bytes{};
            for (auto& byte : bytes)
                byte = static_cast<unsigned char>(random());
            bytes[6] = (bytes[6] & 0x0f) | 0x40;
            bytes[8] = (bytes[8] & 0x3f) | 0x80;
            std::ostringstream out;
            out << std::hex << std::setfill('0');
            for (size_t i = 0; i < bytes.size(); ++i) {
                if (i == 4 || i == 6 || i == 8 || i == 10)
                    out << '-';
                out << std::setw(2) << static_cast<unsigned>(bytes[i]);
            }
            return out.str();
        }

        std::string required_utf8(hid_t object, std::string_view field) {
            if (!has_attribute(object, field))
                throw Error("Missing DIPH5 v3 attribute: " + std::string(field));
            auto value = read_string(object, field);
            if (!valid_utf8(value))
                throw Error("Invalid UTF-8 in DIPH5 v3 attribute: " + std::string(field));
            return value;
        }

        bool has_link(hid_t object, const char* path) {
            const auto result = H5Lexists(object, path, H5P_DEFAULT);
            if (result < 0)
                throw Error(std::string("Unable to inspect DIPH5 v3 path: ") + path);
            return result > 0;
        }

        const char* status_name(ExperimentStatus status) {
            switch (status) {
            case ExperimentStatus::Prepared: return "prepared";
            case ExperimentStatus::Running: return "running";
            case ExperimentStatus::Completed: return "completed";
            case ExperimentStatus::Failed: return "failed";
            case ExperimentStatus::Aborted: return "aborted";
            }
            throw Error("Invalid experiment status");
        }

        ExperimentStatus parse_status(const std::string& value) {
            for (auto status : {ExperimentStatus::Prepared, ExperimentStatus::Running,
                                ExperimentStatus::Completed, ExperimentStatus::Failed, ExperimentStatus::Aborted})
                if (value == status_name(status))
                    return status;
            throw Error("Invalid DIPH5 v3 execution status");
        }

        size_t checked_argv_width(const std::vector<std::string>& args) {
            if (args.empty() || args.size() > 100000)
                throw Error("An execution command needs a bounded, nonempty argv");
            size_t width = 1;
            for (const auto& arg : args) {
                if (!valid_utf8(arg) || arg.size() > 1024 * 1024)
                    throw Error("An execution argument is invalid UTF-8 or too long");
                width = std::max(width, arg.size() + 1);
            }
            if (args.size() > 256ULL * 1024 * 1024 / width)
                throw Error("Execution argv exceeds the archive limit");
            return width;
        }

        void write_argv(hid_t group, const char* name, const std::vector<std::string>& args) {
            const auto width = checked_argv_width(args);
            std::vector<char> bytes(args.size() * width, '\0');
            for (size_t i = 0; i < args.size(); ++i)
                std::copy(args[i].begin(), args[i].end(), bytes.begin() + static_cast<ptrdiff_t>(i * width));
            const hsize_t count = args.size();
            Id space(H5Screate_simple(1, &count, nullptr), H5Sclose, "Unable to create argv shape");
            Id type(H5Tcopy(H5T_C_S1), H5Tclose, "Unable to create argv type");
            check(H5Tset_size(type, width), "Unable to size argv type");
            check(H5Tset_cset(type, H5T_CSET_UTF8), "Unable to set argv encoding");
            Id dataset(H5Dcreate2(group, name, type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                       H5Dclose, "Unable to create execution argv");
            check(H5Dwrite(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, bytes.data()),
                  "Unable to write execution argv");
        }

        std::vector<std::string> read_argv(hid_t group, const char* name) {
            Id dataset(H5Dopen2(group, name, H5P_DEFAULT), H5Dclose, "Missing execution argv");
            Id type(H5Dget_type(dataset), H5Tclose, "Unable to inspect execution argv type");
            Id space(H5Dget_space(dataset), H5Sclose, "Unable to inspect execution argv shape");
            if (H5Tget_class(type) != H5T_STRING || H5Tis_variable_str(type) != 0 ||
                H5Sget_simple_extent_ndims(space) != 1)
                throw Error("Invalid execution argv dataset");
            hsize_t count = 0;
            check(H5Sget_simple_extent_dims(space, &count, nullptr), "Unable to read execution argv shape");
            const auto width = H5Tget_size(type);
            if (!count || count > 100000 || !width || width > 1024 * 1024 ||
                count > 256ULL * 1024 * 1024 / width)
                throw Error("Execution argv exceeds the reader limit");
            std::vector<char> bytes(static_cast<size_t>(count) * width, '\0');
            check(H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, bytes.data()),
                  "Unable to read execution argv");
            std::vector<std::string> args;
            args.reserve(static_cast<size_t>(count));
            for (size_t i = 0; i < count; ++i) {
                const char* begin = bytes.data() + i * width;
                const char* end = std::find(begin, begin + width, '\0');
                if (end == begin + width || std::any_of(end + 1, begin + width,
                                                         [](char byte) { return byte != '\0'; }))
                    throw Error("Execution argv has invalid string padding");
                args.emplace_back(begin, end);
                if (!valid_utf8(args.back()))
                    throw Error("Invalid UTF-8 in execution argv");
            }
            return args;
        }

        std::string hash_file(const std::filesystem::path& path) {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
                throw Error("Unable to read executable for SHA-256");
            snt::Sha256 hash;
            std::array<char, 64 * 1024> buffer{};
            while (stream) {
                stream.read(buffer.data(), buffer.size());
                hash.update(buffer.data(), static_cast<size_t>(stream.gcount()));
            }
            if (!stream.eof())
                throw Error("Unable to finish executable SHA-256");
            return hash.finish();
        }

        Id open_experiment_file(const std::filesystem::path& file) {
            H5Eset_auto2(H5E_DEFAULT, nullptr, nullptr);
            return Id(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose,
                      "Unable to update DIPH5 experiment");
        }
} // namespace snt::dip::hdf5::detail
