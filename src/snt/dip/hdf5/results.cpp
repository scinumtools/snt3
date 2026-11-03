#include "io_private.h"
#include "../input/utf8.h"

#include <algorithm>
#include <cstring>
#include <cmath>
#include <fstream>
#include <limits>
#include <snt/dip/environment.h>
#include <snt/sha256.h>
#include <unordered_set>

namespace snt::dip::hdf5 {
    using namespace detail;
    namespace detail {
        void append_u64(std::string& bytes, uint64_t value) {
            for (unsigned i = 0; i < 8; ++i)
                bytes.push_back(static_cast<char>((value >> (8 * i)) & 0xff));
        }

        void append_text(std::string& bytes, const std::string& value) {
            append_u64(bytes, value.size());
            bytes.append(value);
        }

        struct ResultType {
            hid_t disk;
            hid_t memory;
            size_t width;
            enum class Category { Signed, Unsigned, Floating } category;
        };

        ResultType result_type(const std::string& dtype) {
            using C = ResultType::Category;
            if (dtype == "bool") return {H5T_STD_U8LE, H5T_NATIVE_UINT64, 1, C::Unsigned};
            if (dtype == "char") return {H5T_STD_I8LE, H5T_NATIVE_INT64, 1, C::Signed};
            if (dtype == "int8") return {H5T_STD_I8LE, H5T_NATIVE_INT64, 1, C::Signed};
            if (dtype == "int16") return {H5T_STD_I16LE, H5T_NATIVE_INT64, 2, C::Signed};
            if (dtype == "int32") return {H5T_STD_I32LE, H5T_NATIVE_INT64, 4, C::Signed};
            if (dtype == "int64") return {H5T_STD_I64LE, H5T_NATIVE_INT64, 8, C::Signed};
            if (dtype == "uint8") return {H5T_STD_U8LE, H5T_NATIVE_UINT64, 1, C::Unsigned};
            if (dtype == "uint16") return {H5T_STD_U16LE, H5T_NATIVE_UINT64, 2, C::Unsigned};
            if (dtype == "uint32") return {H5T_STD_U32LE, H5T_NATIVE_UINT64, 4, C::Unsigned};
            if (dtype == "uint64") return {H5T_STD_U64LE, H5T_NATIVE_UINT64, 8, C::Unsigned};
            if (dtype == "float32") return {H5T_IEEE_F32LE, H5T_NATIVE_DOUBLE, 4, C::Floating};
            if (dtype == "float64") return {H5T_IEEE_F64LE, H5T_NATIVE_DOUBLE, 8, C::Floating};
            throw Error("Unsupported result data type: " + dtype);
        }

        void validate_values(const ExperimentValues& values, const ResultType& type) {
            using C = ResultType::Category;
            if (type.category == C::Signed) {
                const auto* items = std::get_if<std::vector<int64_t>>(&values);
                if (!items) throw Error("Signed result requires signed integer values");
                if (type.width < 8) {
                    const auto limit = int64_t{1} << (type.width * 8 - 1);
                    for (auto value : *items)
                        if (value < -limit || value >= limit) throw Error("Signed result value out of range");
                }
            } else if (type.category == C::Unsigned) {
                const auto* items = std::get_if<std::vector<uint64_t>>(&values);
                if (!items) throw Error("Unsigned result requires unsigned integer values");
                if (type.width < 8) {
                    const auto limit = uint64_t{1} << (type.width * 8);
                    for (auto value : *items)
                        if (value >= limit) throw Error("Unsigned result value out of range");
                }
            } else {
                const auto* items = std::get_if<std::vector<double>>(&values);
                if (!items) throw Error("Floating result requires floating-point values");
                if (type.width == 4)
                    for (auto value : *items)
                        if (std::isfinite(value) && std::abs(value) > std::numeric_limits<float>::max())
                            throw Error("float32 result value out of range");
            }
        }

        std::string result_hash(hid_t data, const std::string& dtype, const ResultType& type,
                                const std::vector<uint64_t>& shape, const std::string& units,
                                const std::vector<std::string>& dimensions, size_t count) {
            std::string bytes = "diph5-result-v1";
            append_text(bytes, dtype);
            append_u64(bytes, shape.size());
            for (auto extent : shape) append_u64(bytes, extent);
            append_text(bytes, units);
            append_u64(bytes, dimensions.size());
            for (const auto& dimension : dimensions) append_text(bytes, dimension);
            std::vector<char> encoded(checked_product(count, type.width, "result data bytes"));
            if (count) check(H5Dread(data, type.disk, H5S_ALL, H5S_ALL, H5P_DEFAULT, encoded.data()),
                             "Unable to read canonical result bytes");
            if (!encoded.empty()) bytes.append(encoded.data(), encoded.size());
            return snt::sha256(bytes);
        }

        std::vector<ExperimentResult> inspect_results(hid_t archive) {
            std::vector<ExperimentResult> all;
            if (!has_link(archive, "/results")) {
                if (required_utf8(archive, "_DIPH5_State") == "completed")
                    throw Error("A completed run needs a finalized result inventory");
                return all;
            }
            Id root(H5Gopen2(archive, "/results", H5P_DEFAULT), H5Gclose, "Invalid results group");
            Id manifest(H5Gopen2(root, "manifest", H5P_DEFAULT), H5Gclose, "Missing result manifest");
            const auto expected = required_scalar<uint64_t>(manifest, "result_count", H5T_NATIVE_UINT64);
            const auto finalized = required_scalar<uint8_t>(manifest, "finalized", H5T_NATIVE_UINT8);
            if (finalized > 1 || expected > 100000 ||
                (required_utf8(archive, "_DIPH5_State") == "completed" && !finalized))
                throw Error("Invalid result manifest finalization");
            if (finalized && required_utf8(manifest, "finalized_at").empty())
                throw Error("Finalized results have no timestamp");
            const auto run_id = required_utf8(archive, "_DIPH5_Record_Id");
            std::unordered_set<std::string> ids;
            for (const auto category : {"datasets", "artifacts", "diagnostics"}) {
                if (!has_link(root, category)) continue;
                Id group(H5Gopen2(root, category, H5P_DEFAULT), H5Gclose, "Invalid result category");
                const auto count = checked_size(object_count(group), "result count");
                if (count > 100000) throw Error("Too many results");
                for (size_t i = 0; i < count; ++i) {
                    const auto id = object_name(group, i);
                    Id entry(H5Gopen2(group, id.c_str(), H5P_DEFAULT), H5Gclose, "Invalid result entry");
                    ExperimentResult result;
                    result.result_id = required_utf8(entry, "result_id");
                    result.name = required_utf8(entry, "name");
                    result.kind = required_utf8(entry, "kind");
                    result.storage = required_utf8(entry, "storage");
                    result.status = required_utf8(entry, "status");
                    if (id != result.result_id || !ids.insert(id).second || result.name.empty() ||
                        required_utf8(entry, "run_id") != run_id ||
                        (result.status != "registered" && result.status != "writing" &&
                         result.status != "complete" && result.status != "failed" && result.status != "missing"))
                        throw Error("Invalid result identity or status");
                    if (std::string(category) == "artifacts") {
                        if (result.kind != "artifact" || result.storage != "external")
                            throw Error("Invalid artifact identity");
                        result.path_or_uri = required_utf8(entry, "path_or_uri");
                        result.sha256 = read_string(entry, "sha256");
                        result.verification = required_utf8(entry, "verification");
                        if (has_attribute(entry, "size_bytes"))
                            result.size_bytes = required_scalar<uint64_t>(entry, "size_bytes", H5T_NATIVE_UINT64);
                        if (result.status == "complete" &&
                            (result.sha256.size() != 64 || !has_attribute(entry, "size_bytes")))
                            throw Error("Complete artifact lacks a digest or size");
                    } else {
                        if (result.kind != (std::string(category) == "datasets" ? "dataset" : "diagnostic") ||
                            result.storage != "embedded")
                            throw Error("Invalid embedded result identity");
                        result.units = read_string(entry, "units");
                        result.description = read_string(entry, "description");
                        result.sha256 = required_utf8(entry, "data_sha256");
                        result.logical_dtype = required_utf8(entry, "logical_dtype");
                        const auto result_dtype = result_type(result.logical_dtype);
                        if (has_link(entry, "dimensions"))
                            result.dimensions = read_argv(entry, "dimensions");
                        Id data(H5Dopen2(entry, "data", H5P_DEFAULT), H5Dclose, "Missing result data");
                        Id type(H5Dget_type(data), H5Tclose, "Invalid result data type");
                        Id space(H5Dget_space(data), H5Sclose, "Invalid result data shape");
                        const auto expected_class = result_dtype.category == ResultType::Category::Floating
                                                        ? H5T_FLOAT : H5T_INTEGER;
                        if (H5Tget_class(type) != expected_class || H5Tget_size(type) != result_dtype.width ||
                            (expected_class == H5T_INTEGER &&
                             H5Tget_sign(type) != (result_dtype.category == ResultType::Category::Signed
                                                       ? H5T_SGN_2 : H5T_SGN_NONE)))
                            throw Error("Result data type does not match logical dtype");
                        const auto rank = H5Sget_simple_extent_ndims(space);
                        if (rank < 0 || rank > 8) throw Error("Unsupported result rank");
                        std::vector<hsize_t> extents(static_cast<size_t>(rank));
                        if (rank) check(H5Sget_simple_extent_dims(space, extents.data(), nullptr), "Invalid result shape");
                        uint64_t count_values = 1;
                        for (auto extent : extents) {
                            if (extent > 100000000 || count_values > 100000000 / std::max<hsize_t>(extent, 1))
                                throw Error("Result exceeds reader limit");
                            count_values *= extent;
                            result.shape.push_back(extent);
                        }
                        if (result_dtype.category == ResultType::Category::Signed)
                            result.typed_values = std::vector<int64_t>(static_cast<size_t>(count_values));
                        else if (result_dtype.category == ResultType::Category::Unsigned)
                            result.typed_values = std::vector<uint64_t>(static_cast<size_t>(count_values));
                        else
                            result.typed_values = std::vector<double>(static_cast<size_t>(count_values));
                        if (count_values) std::visit([&](auto& values) {
                            check(H5Dread(data, result_dtype.memory, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                                          values.data()), "Unable to read result data");
                        }, result.typed_values);
                        if (result.logical_dtype == "bool")
                            for (auto value : std::get<std::vector<uint64_t>>(result.typed_values))
                                if (value > 1) throw Error("Invalid boolean result value");
                        if (const auto* values = std::get_if<std::vector<double>>(&result.typed_values))
                            result.values = *values;
                        if (!result.dimensions.empty() && result.dimensions.size() != result.shape.size())
                            throw Error("Result dimension names do not match its rank");
                        if (result_hash(data, result.logical_dtype, result_dtype, result.shape, result.units,
                                        result.dimensions, static_cast<size_t>(count_values)) != result.sha256)
                            throw IntegrityError("Embedded result checksum mismatch");
                        if (std::string(category) == "diagnostics")
                            result.verification = required_utf8(entry, "diagnostic_category");
                        if (result.kind == "diagnostic" && result.verification != "observed" &&
                            result.verification != "judgment")
                            throw Error("Invalid diagnostic category");
                    }
                    all.push_back(std::move(result));
                }
            }
            if (all.size() != expected)
                throw Error("Result manifest count does not match inventory: " + std::to_string(expected) +
                            " entries declared, " + std::to_string(all.size()) + " found");
            if (!has_link(manifest, "entries")) {
                if (expected) throw Error("Result manifest has no entries");
            } else {
                Id entries(H5Gopen2(manifest, "entries", H5P_DEFAULT), H5Gclose, "Invalid manifest entries");
                if (object_count(entries) != expected) throw Error("Result manifest entry count mismatch");
                for (const auto& result : all) {
                    if (!has_link(entries, result.result_id.c_str())) throw Error("Result is absent from manifest");
                    Id entry(H5Gopen2(entries, result.result_id.c_str(), H5P_DEFAULT), H5Gclose,
                             "Invalid result manifest entry");
                    if (required_utf8(entry, "name") != result.name ||
                        required_utf8(entry, "category") !=
                            (result.kind == "artifact" ? "artifacts" : result.kind == "diagnostic" ? "diagnostics" : "datasets") ||
                        required_utf8(entry, "status") != result.status)
                        throw Error("Result manifest entry disagrees with result");
                    const auto path = "/results/" +
                        std::string(result.kind == "artifact" ? "artifacts" :
                                    result.kind == "diagnostic" ? "diagnostics" : "datasets") +
                        "/" + result.result_id;
                    if (required_utf8(entry, "path") != path)
                        throw Error("Result manifest path disagrees with result");
                }
            }
            return all;
        }

    } // namespace detail
    namespace {
        Id ensure_results(hid_t archive) {
            if (!has_link(archive, "/results")) {
                Id root(H5Gcreate2(archive, "/results", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                        H5Gclose, "Unable to create results group");
                Id manifest(H5Gcreate2(root, "manifest", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                            H5Gclose, "Unable to create result manifest");
                write_scalar<uint64_t>(manifest, "result_count", H5T_NATIVE_UINT64, 0);
                write_scalar<uint8_t>(manifest, "finalized", H5T_NATIVE_UINT8, 0);
                Id entries(H5Gcreate2(manifest, "entries", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                           H5Gclose, "Unable to create manifest entries");
                return root;
            }
            return Id(H5Gopen2(archive, "/results", H5P_DEFAULT), H5Gclose, "Unable to open results group");
        }

        void register_result(hid_t root, const std::string& id, const std::string& name,
                             const std::string& category, const std::string& status) {
            Id manifest(H5Gopen2(root, "manifest", H5P_DEFAULT), H5Gclose, "Missing result manifest");
            if (required_scalar<uint8_t>(manifest, "finalized", H5T_NATIVE_UINT8))
                throw Error("Result inventory is finalized");
            Id entries(H5Gopen2(manifest, "entries", H5P_DEFAULT), H5Gclose, "Missing manifest entries");
            Id item(H5Gcreate2(entries, id.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Gclose, "Unable to register result");
            write_string(item, "name", name);
            write_string(item, "category", category);
            write_string(item, "status", status);
            const auto count = required_scalar<uint64_t>(manifest, "result_count", H5T_NATIVE_UINT64);
            write_string(item, "path", "/results/" + category + "/" + id);
            write_scalar<uint64_t>(manifest, "result_count", H5T_NATIVE_UINT64, count + 1);
        }

        void require_unfinalized(hid_t root) {
            Id manifest(H5Gopen2(root, "manifest", H5P_DEFAULT), H5Gclose, "Missing result manifest");
            if (required_scalar<uint8_t>(manifest, "finalized", H5T_NATIVE_UINT8))
                throw Error("Result inventory is finalized");
        }

        void require_running_results(const std::filesystem::path& file) {
            if (read_experiment(file).status != ExperimentStatus::Running)
                throw Error("Results can only be registered while the experiment is running");
        }
    }

    std::string add_result_dataset(const std::filesystem::path& file, const std::string& name,
                                   const ExperimentValues& values, const std::string& dtype,
                                   const std::vector<uint64_t>& shape,
                                   const std::string& units, const std::vector<std::string>& dimensions,
                                   const std::string& description, const std::string& category) {
        require_running_results(file);
        if (name.empty() || !valid_utf8(name) || !valid_utf8(units) || !valid_utf8(description) ||
            shape.size() > 8 || (!dimensions.empty() && dimensions.size() != shape.size()))
            throw Error("Invalid result name, metadata, or shape");
        uint64_t count = 1;
        for (auto extent : shape) {
            if (extent > 100000000 || count > 100000000 / std::max<uint64_t>(extent, 1))
                throw Error("Result exceeds writer limit");
            count *= extent;
        }
        const auto result_dtype = result_type(dtype);
        validate_values(values, result_dtype);
        if (dtype == "bool")
            for (auto value : std::get<std::vector<uint64_t>>(values))
                if (value > 1) throw Error("Boolean result values must be 0 or 1");
        const auto value_count = std::visit([](const auto& data) { return data.size(); }, values);
        if (count != value_count) throw Error("Result values do not match shape");
        for (const auto& dimension : dimensions)
            if (dimension.empty() || !valid_utf8(dimension)) throw Error("Invalid result dimension name");
        const bool diagnostic = category.rfind("diagnostics:", 0) == 0;
        if (category != "datasets" && !diagnostic) throw Error("Invalid result category");
        const auto group_name = diagnostic ? "diagnostics" : "datasets";
        const auto id = uuid();
        Id archive = open_experiment_file(file);
        Id root = ensure_results(archive);
        require_unfinalized(root);
        if (!has_link(root, group_name)) {
            Id created(H5Gcreate2(root, group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                       H5Gclose, "Unable to create result category");
        }
        Id group(H5Gopen2(root, group_name, H5P_DEFAULT), H5Gclose, "Unable to open result category");
        Id entry(H5Gcreate2(group, id.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                 H5Gclose, "Unable to create result");
        write_string(entry, "result_id", id);
        write_string(entry, "name", name);
        write_string(entry, "kind", diagnostic ? "diagnostic" : "dataset");
        write_string(entry, "storage", "embedded");
        write_string(entry, "status", "complete");
        write_string(entry, "run_id", required_utf8(archive, "_DIPH5_Record_Id"));
        write_string(entry, "logical_dtype", dtype);
        write_string(entry, "units", units);
        write_string(entry, "description", description);
        if (diagnostic) write_string(entry, "diagnostic_category", category.substr(12));
        if (!dimensions.empty()) write_argv(entry, "dimensions", dimensions);
        std::vector<hsize_t> extents(shape.begin(), shape.end());
        Id space(shape.empty() ? H5Screate(H5S_SCALAR) :
                 H5Screate_simple(static_cast<int>(shape.size()), extents.data(), nullptr),
                 H5Sclose, "Unable to create result dataspace");
        Id data(H5Dcreate2(entry, "data", result_dtype.disk, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                H5Dclose, "Unable to create result dataset");
        if (value_count) std::visit([&](const auto& items) {
            check(H5Dwrite(data, result_dtype.memory, H5S_ALL, H5S_ALL, H5P_DEFAULT, items.data()),
                  "Unable to write result data");
        }, values);
        write_string(entry, "data_sha256", result_hash(data, dtype, result_dtype, shape, units, dimensions, value_count));
        register_result(root, id, name, group_name, "complete");
        check(H5Fflush(archive, H5F_SCOPE_GLOBAL), "Unable to flush result");
        return id;
    }

    std::string attach_result_artifact(const std::filesystem::path& file, const std::filesystem::path& artifact,
                                       const std::string& name, const std::string& format) {
        require_running_results(file);
        if (name.empty() || !valid_utf8(name) || !valid_utf8(format))
            throw Error("Invalid artifact metadata");
        const auto absolute = std::filesystem::absolute(artifact).lexically_normal();
        std::error_code error;
        const bool present = std::filesystem::is_regular_file(absolute, error);
        const auto relative = absolute.lexically_relative(std::filesystem::absolute(file).parent_path());
        const auto recorded = !relative.empty() && *relative.begin() != std::filesystem::path("..")
                                  ? relative.generic_string() : absolute.generic_string();
        const auto id = uuid();
        Id archive = open_experiment_file(file);
        Id root = ensure_results(archive);
        require_unfinalized(root);
        if (!has_link(root, "artifacts")) {
            Id created(H5Gcreate2(root, "artifacts", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                       H5Gclose, "Unable to create artifacts group");
        }
        Id group(H5Gopen2(root, "artifacts", H5P_DEFAULT), H5Gclose, "Unable to open artifacts group");
        Id entry(H5Gcreate2(group, id.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                 H5Gclose, "Unable to create artifact entry");
        write_string(entry, "result_id", id);
        write_string(entry, "name", name);
        write_string(entry, "kind", "artifact");
        write_string(entry, "storage", "external");
        write_string(entry, "run_id", required_utf8(archive, "_DIPH5_Record_Id"));
        write_string(entry, "path_or_uri", recorded);
        if (!format.empty()) write_string(entry, "format", format);
        write_string(entry, "status", present ? "complete" : "missing");
        write_string(entry, "verification", present ? "verified" : "missing");
        if (present) {
            write_scalar<uint64_t>(entry, "size_bytes", H5T_NATIVE_UINT64,
                                   std::filesystem::file_size(absolute));
            write_string(entry, "sha256", hash_file(absolute));
        }
        register_result(root, id, name, "artifacts", present ? "complete" : "missing");
        check(H5Fflush(archive, H5F_SCOPE_GLOBAL), "Unable to flush artifact");
        return id;
    }

    std::vector<ExperimentResult> read_results(const std::filesystem::path& file) {
        read_experiment(file);
        H5Eset_auto2(H5E_DEFAULT, nullptr, nullptr);
        Id archive(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose,
                   "Unable to open experiment results");
        return inspect_results(archive);
    }

    void finalize_results(const std::filesystem::path& file, bool imported) {
        if (imported) {
            if (read_experiment(file).status != ExperimentStatus::Prepared)
                throw Error("Only a prepared imported experiment can finalize an empty inventory");
        } else {
            require_running_results(file);
        }
        const auto verification = verify_experiment(file);
        if (verification.structural != "pass" || verification.integrity != "pass" ||
            verification.external_artifacts == "fail")
            throw Error("Cannot finalize results: archive or artifact verification failed");
        Id archive = open_experiment_file(file);
        Id root = ensure_results(archive);
        Id manifest(H5Gopen2(root, "manifest", H5P_DEFAULT), H5Gclose, "Missing result manifest");
        if (required_scalar<uint8_t>(manifest, "finalized", H5T_NATIVE_UINT8))
            throw Error("Result inventory is already finalized");
        // An empty inventory is still an inventory. Seal it before calling the run complete.
        inspect_results(archive);
        write_string(manifest, "finalized_at", utc_now());
        write_scalar<uint8_t>(manifest, "finalized", H5T_NATIVE_UINT8, 1);
        check(H5Fflush(archive, H5F_SCOPE_GLOBAL), "Unable to finalize result inventory");
    }

    ExperimentVerification verify_experiment(const std::filesystem::path& file) {
        ExperimentVerification verification{"pass", "pass", "pass", "unavailable", {}};
        try {
            Id archive(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose,
                       "Unable to open DIPH5 experiment for fingerprint verification");
            Id provenance(H5Gopen2(archive, "/configuration/provenance", H5P_DEFAULT), H5Gclose,
                          "Missing DIPH5 configuration provenance");
            const auto status = required_utf8(provenance, "fingerprint_status");
            const auto stored = required_utf8(provenance, "configuration_sha256");
            if (status == "pass") {
                const auto actual = configuration_fingerprint(archive);
                verification.configuration_fingerprint = actual && *actual == stored ? "pass" : "fail";
                if (verification.configuration_fingerprint == "fail") {
                    verification.integrity = "fail";
                    verification.details.push_back("Configuration fingerprint mismatch");
                }
            } else if (status != "unavailable" || !stored.empty()) {
                verification.configuration_fingerprint = "fail";
                verification.integrity = "fail";
                verification.details.push_back("Invalid configuration fingerprint status");
            }
        } catch (const std::exception& error) {
            verification.configuration_fingerprint = "fail";
            verification.integrity = "fail";
            verification.details.push_back(error.what());
        }
        std::vector<ExperimentResult> results;
        try {
            results = read_results(file);
        } catch (const IntegrityError& error) {
            verification.integrity = "fail";
            verification.external_artifacts = "unavailable";
            verification.details.push_back(error.what());
            return verification;
        } catch (const std::exception& error) {
            verification.structural = "fail";
            verification.integrity = "fail";
            verification.external_artifacts = "unavailable";
            verification.details.push_back(error.what());
            return verification;
        }
        for (const auto& result : results) {
            if (result.kind != "artifact") continue;
            if (result.status == "missing") {
                if (verification.external_artifacts == "pass") verification.external_artifacts = "unavailable";
                verification.details.push_back(result.name + ": artifact was missing when registered");
                continue;
            }
            const auto path = std::filesystem::path(result.path_or_uri);
            // A path from an archive is a claim, not permission to roam the host filesystem.
            if (path.is_absolute() || result.path_or_uri.find("://") != std::string::npos) {
                if (verification.external_artifacts == "pass") verification.external_artifacts = "unavailable";
                verification.details.push_back(result.name + ": external path needs explicit verification");
                continue;
            }
            if (!path.is_absolute()) {
                for (const auto& part : path)
                    if (part == "..") {
                        verification.external_artifacts = "fail";
                        verification.details.push_back(result.name + ": unsafe relative artifact path");
                        break;
                    }
                if (verification.external_artifacts == "fail") continue;
            }
            const auto archive_directory = std::filesystem::absolute(file).parent_path();
            const auto resolved = archive_directory / path;
            std::error_code error;
            if (!std::filesystem::is_regular_file(resolved, error)) {
                if (verification.external_artifacts == "pass") verification.external_artifacts = "unavailable";
                verification.details.push_back(result.name + ": artifact is missing");
            } else {
                const auto base = std::filesystem::canonical(archive_directory, error);
                const auto actual = std::filesystem::canonical(resolved, error);
                const auto relative = actual.lexically_relative(base);
                if (error || relative.empty() || *relative.begin() == std::filesystem::path("..")) {
                    if (verification.external_artifacts == "pass") verification.external_artifacts = "unavailable";
                    verification.details.push_back(result.name + ": artifact resolves outside archive directory");
                    continue;
                }
                if (result.sha256.empty() || !result.size_bytes ||
                       std::filesystem::file_size(resolved, error) != *result.size_bytes || error ||
                       hash_file(resolved) != result.sha256) {
                    verification.external_artifacts = "fail";
                    verification.details.push_back(result.name + ": artifact checksum mismatch");
                }
            }
        }
        return verification;
    }
} // namespace snt::dip::hdf5
